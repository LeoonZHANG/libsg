#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "vtd-xml/vtdGen.h"
#include "vtd-xml/vtdNav.h"
#include "vtd-xml/autoPilot.h"
#include "vtd-xml/XMLModifier.h"
#include "vtd-xml/xpath.h"
#include <sg/sg.h>
#include <sg/container/xml.h>

#if defined(WIN32)
#define asprintf rpl_asprintf
#endif


static char *make_utf8_string(const wchar_t *in, int inSize) {

	int inIndex = 0;

	char *out = NULL;
	int outSize = 0;
	int outIndex = 0;
	/* first calculate the size of the target string */
	while (inIndex < inSize) {
		wchar_t c = in[inIndex++];
		if (c < 0x0080) {
			outSize += 1;
		} else if (c < 0x0800) {
			outSize += 2;
		} else {
			outSize += 3;
		}
	}

	out = malloc(outSize + 1);
	if (out == NULL) {
		return NULL;
	}
	inIndex = 0;
	while (inIndex < inSize) {
		wchar_t c = in[inIndex++];
		if (c < 0x080) {
			out[outIndex++] = (unsigned char) c;
		} else if (c < 0x800) {
			out[outIndex++] = 0xc0 | (c >> 6);
			out[outIndex++] = 0x80 | (c & 0x3f);
		} else {
			out[outIndex++] = 0xe0 | (c >> 12);
			out[outIndex++] = 0x80 | ((c >> 6) & 0x3f);
			out[outIndex++] = 0x80 | (c & 0x3f);
		}
	}

	out[outIndex] = 0x00;

	return out;
}

static wchar_t *make_unicode_string(const char *in, int inSize) {
	int inIndex = 0;

	wchar_t *out = NULL;
	int outIndex = 0;
	int outSize = 0;
	/* first calculate the size of the target string */

	while (inIndex < inSize) {
		unsigned char c = in[inIndex];
		if ((c & 0x80) == 0) {
			inIndex += 1;
		} else if ((c & 0xe0) == 0xe0) {
			inIndex += 3;
		} else {
			inIndex += 2;
		}
		outSize++;
	}

	out = malloc((outSize + 1) * sizeof(wchar_t));
	if (out == NULL) {
		return NULL;
	}
	inIndex = 0;
	while (inIndex < inSize) {
		unsigned char c = in[inIndex++];
		if ((c & 0x80) == 0) {
			out[outIndex++] = c;
		} else if ((c & 0xe0) == 0xe0) {
			out[outIndex] = (c & 0x1F) << 12;
			c = in[inIndex++];
			out[outIndex] |= (c & 0x3F) << 6;
			c = in[inIndex++];
			out[outIndex++] |= (c & 0x3F);
		} else {
			out[outIndex] = (c & 0x3F) << 6;
			c = in[inIndex++];
			out[outIndex++] |= (c & 0x3F);
		}

	}

	out[outIndex] = 0;

	return out;
}



/********************************** vtdxml **********************************/
static int vtdxml_get_string(VTDNav *vn, const char *xpath, char **value);
static int vtdxml_set_string(VTDNav *vn, const char *xpath, char *value);
static VTDNav * vtdxml_create_doc(const char *buffer, size_t length);
static void vtdxml_destroy_doc(VTDNav *vn);
//返回-1表示错误
static int vtdxml_doc_to_str(VTDNav  *vn, char **str);
//返回-1表示错误，>=0表示为成员数
static int vtdxml_get_member_size(VTDNav  *vn, const char *xpath);


// using thread local storage in 2.10
_thread struct exception_context the_exception_context[1];

static inline void print_exception(const exception *e) {
	printf("type=%d,sub_type=%d,msg=%s,sub_msg=%s\n", e->et, e->subtype, e->msg,
			e->sub_msg);
}
//如果需要支持名字空间，需要先定义别名，然后在xpath中就可以别名了，如：/ns1:root/ns1:a
	//nsMap={"ns1","url1","ns2","url2"}
static AutoPilot *select_xpath(VTDNav *vn, const char *xpath,char *nsMap[],int nsMapSize) {

	exception e;
	AutoPilot *ap = NULL;
	AutoPilot *apTemp = NULL;
	UCSChar *xpathTemp = NULL;


	UCSChar *nsMapTemp = (UCSChar *) calloc(nsMapSize, sizeof(UCSChar));
	Try
			{
				apTemp = createAutoPilot(vn);
				if (apTemp == NULL) {
					goto clean;
				}

				for(int i=0;i<nsMapSize;){
					//不能够在这里创建，必须在调用的地方创建，否则没有办法释放对象
					UCSChar *ns= make_unicode_string(nsMap[i],strlen(nsMap[i]));
					UCSChar *url= make_unicode_string(nsMap[i+1],strlen(nsMap[i+1]));
					nsMapTemp[i]=ns;
					nsMapTemp[i+1]=url;
					i+=2;
					declareXPathNameSpace(apTemp,ns,url);

				}

				xpathTemp = make_unicode_string(xpath, strlen(xpath));
				if (xpathTemp == NULL || !selectXPath(apTemp, xpathTemp)) {
					goto clean;
				}
				ap = apTemp;
				apTemp = NULL;
			}Catch
		(e)
	{
		print_exception(&e);
	}

	clean:
	//
	freeAutoPilot(apTemp);
	free(xpathTemp);
	for(int i=0;i<nsMapSize;i++){
		UCSChar *s = nsMapTemp[i];
		free(s);
	}
	free(nsMapTemp);
	return ap;

}

static int vtdxml_get_string(VTDNav *vn, const char *xpath, char **value) {
	exception e;
	AutoPilot *ap = NULL;
	UCSChar * tempValue = NULL;
	int error = -1;
	Try
			{
				ap = select_xpath(vn, xpath,NULL,0);
				if (ap == NULL) {
					goto clean;
				}
				tempValue = evalXPathToString(ap);
				if (tempValue != NULL) {
					char * str = make_utf8_string(tempValue, wcslen(tempValue));
					if (str != NULL) {
						*value = str;
						error = 0;
					}
				} else {
					//*value = NULL;
				}

			}Catch
		(e)
	{
		print_exception(&e);

	}

	clean: freeAutoPilot(ap);
	free(tempValue);
	return error;
}

static int vtdxml_set_string(VTDNav *vn, const char *xpath, char *value) {

	exception e;
	AutoPilot *ap = NULL;
	int error = -1;
	Try
			{
				ap = select_xpath(vn,xpath,NULL,0);
				if(ap==NULL){
					goto clean;
				}
				int index = evalXPath(ap);
				if (index == -1) {
					goto clean;
				}

				//printf("token type=%d, length=%d,size=%d\n",getTokenType(vn,index),getTokenLength(vn,index),strlen(value));
				if ( !overWrite(vn, index, value, 0, strlen(value))) {
					//如果不能够写，

					goto clean;
				}
				error = 0;
			}Catch
		(e)
	{
		print_exception(&e);
	}

	clean:
	freeAutoPilot(ap);
	return error;
}

static VTDNav * vtdxml_create_doc(const char *buffer, size_t size) {
	exception e;
	VTDGen *vg = NULL;
	VTDNav *vn = NULL;
	Try
			{
				vg = createVTDGen();
				if (vg == NULL) {
					goto clean;
				}
				setDoc(vg, buffer, size);
				parse(vg, TRUE);
				vn = getNav(vg);
			}Catch
		(e)
	{
		print_exception(&e);
	}

	clean: freeVTDGen(vg);

	return vn;
}

static void vtdxml_destroy_doc(VTDNav *vn) {
	//如果是创建的就需要释放，如果是外部传入的就不需要释放
	//if(vn->XMLDoc){
		//free(vn->XMLDoc)
	//}
	if(vn!=NULL){
		freeVTDNav(vn);
	}

}

//返回-1表示失败
static int vtdxml_get_member_size(VTDNav *vn, const char *xpath) {
	exception e;
	AutoPilot *ap = NULL;
	int size=-1;
	Try
			{

				ap = select_xpath(vn,xpath,NULL,0);
				if(ap==NULL){
					goto clean;
				}

				while (evalXPath(ap) != -1) {
					if(size<0){
						size=1;
					}else{
						size++;
					}
				}
			}Catch
		(e)
	{
		print_exception(&e);

	}

	clean:
	freeAutoPilot(ap);
	return size;
}

/* 不要忘记使用xml_free_string释放xml_str_t */
static int vtdxml_doc_to_str(VTDNav *vn,  char ** str) {
	exception e;
	Try
			{
				//返回的XMLDoc,为内部的，不需要释放
				char *from = getXML(vn);
				//返回的XMLDoc因为没有使用'\0'结尾，不能够使用strlen(XMLDoc)来计算长度
				//可能会返回多余的字符
				int docLen = vn->docLen;
				char *to = malloc(docLen + 1);

				if (to == NULL) {
					return -1;
				}

				strncpy(to, vn->docOffset + from, docLen);

				to[docLen] = '\0';
				*str = to;

				return 0;
			}Catch
		(e)
	{
		print_exception(&e);

	}
			return -1;
}


/********************************** libxml2 **********************************/

/**
 * 解析的3个对象
 * xmlDoc => xmlXPathContext => xmlXPathObject
 * 指针类型为
 * xmlDocPtr => xmlXPathContextPtr => xmlXPathObjectPtr
 * 创建函数
 * xmlParseFile => xmlXPathNewContext(doc) => xmlXPathEvalExpression(xpath,context)
 * 释放函数
 * xmlFreeDoc => xmlXPathFreeContext => xmlXPathFreeObject
 */

static int create_xpath(xmlDocPtr doc, const char *xpath, xmlXPathContextPtr *ctx,
		xmlXPathObjectPtr *xpathObj,char *nsMap[],int nsMapSize) {
	xmlXPathContextPtr ctxPtr = NULL;
	xmlXPathObjectPtr objPtr = NULL;

	//ctxPtr = xmlXPtrNewContext(doc, NULL, NULL);
	ctxPtr = xmlXPathNewContext(doc);
	if (ctxPtr == NULL) {
		return -1;
	}
	//ctxPtr->node = xmlDocGetRootElement(doc);

	for(int i=0;i<nsMapSize;){
		xmlXPathRegisterNs(BAD_CAST ctxPtr,nsMap[i++],BAD_CAST nsMap[i++]);
	}

	//res = xmlXPathEvalExpression(xpath, ctx);
	objPtr = xmlXPathEval(BAD_CAST xpath, ctxPtr);
	if (objPtr == NULL) {
		xmlXPathFreeContext(ctxPtr);
		return -1;
	}

	*ctx = ctxPtr;
	*xpathObj = objPtr;
	return 0;

}

static void libxml_init();
static void libxml_free();
static int libxml_get_string(xmlDocPtr doc,const char *xpath,char **value);
static int libxml_set_string(xmlDocPtr doc,const char *xpath,char *value);
static xmlDocPtr  libxml_create_doc(const char *buffer,size_t length);
static void libxml_destroy_doc(xmlDocPtr doc);
//返回-1表示错误
static int libxml_doc_to_str(xmlDocPtr doc,char **str);
//返回-1表示错误，>=0表示为成员数
static int libxml_get_member_size(xmlDocPtr doc,const char *xpath);

static void libxml_init() {
	xmlInitParser();
	LIBXML_TEST_VERSION
}

static void libxml_free()
{

	/* Shutdown libxml */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
}

static int libxml_set_string(xmlDocPtr doc, const char *xpath, char *value)
{
	xmlXPathObjectPtr xpathObj = NULL;
	xmlXPathContextPtr ctx = NULL;

	if (create_xpath(doc,xpath, &ctx, &xpathObj,NULL,0) != 0) {
		return -1;
	}
	xmlNodeSetPtr nodes = xpathObj->nodesetval;
	int size = (nodes) ? nodes->nodeNr : 0;

	/*
	 * NOTE: the nodes are processed in reverse order, i.e. reverse document
	 *       order because xmlNodeSetContent can actually free up descendant
	 *       of the node and such nodes may have been selected too ! Handling
	 *       in reverse order ensure that descendant are accessed first, before
	 *       they get removed. Mixing XPath and modifications on a tree must be
	 *       done carefully !
	 */
	for (int i = size - 1; i >= 0; i--) {

		xmlNodeSetContent(nodes->nodeTab[i], value);


		/*
		 * All the elements returned by an XPath query are pointers to
		 * elements from the tree *except* namespace nodes where the XPath
		 * semantic is different from the implementation in libxml2 tree.
		 * As a result when a returned node set is freed when
		 * xmlXPathFreeObject() is called, that routine must check the
		 * element type. But node from the returned set may have been removed
		 * by xmlNodeSetContent() resulting in access to freed data.
		 * This can be exercised by running
		 *       valgrind xpath2 test3.xml '//discarded' discarded
		 * There is 2 ways around it:
		 *   - make a copy of the pointers to the nodes from the result set
		 *     then call xmlXPathFreeObject() and then modify the nodes
		 * or
		 *   - remove the reference to the modified nodes from the node set
		 *     as they are processed, if they are not namespace nodes.
		 */
		if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
			nodes->nodeTab[i] = NULL;
	}

	xmlXPathFreeContext(ctx);
	xmlXPathFreeObject(xpathObj);

	//如果没有匹配的节点，返回-1表示失败
	return size==0?-1:0;
}

static int libxml_get_string(xmlDocPtr doc, const char *xpath, char **value) {
	xmlXPathObjectPtr xpathObj;
	xmlXPathContextPtr ctx;

    if(create_xpath(doc,xpath,&ctx,&xpathObj,NULL,0)!=0){
    	return -1;
    }


	*value = xmlXPathCastToString(xpathObj);

    xmlXPathFreeObject(xpathObj);
	xmlXPathFreeContext(ctx);
	return 0;
}

static xmlDocPtr libxml_create_doc(const char *buffer, size_t size) {
	if (buffer == NULL || size == 0) {
		buffer = "<?xml version='1.0' encoding='UTF-8'?><root></root>";
		size = strlen(buffer);
		//doc = xmlNewDoc(BAD_CAST "1.0");
		// n = xmlNewNode(NULL, BAD_CAST "root");
		// xmlNodeSetContent(n, BAD_CAST "content");
		// xmlDocSetRootElement(doc, n);
	} else {

	}
	return xmlReadMemory(buffer, size, NULL, "utf-8", XML_PARSE_COMPACT);

}
static void libxml_destroy_doc(xmlDocPtr doc) {
	xmlFreeDoc(doc);
}
//返回-1表示错误
static int libxml_doc_to_str(xmlDocPtr doc, char **str) {
	int size = 0;
	//xmlDocDumpFormatMemory(doc, str, &size, 1);
	xmlDocDumpFormatMemoryEnc(doc, str, &size, "utf-8", 1);

	return 0;
}
//返回-1表示错误，>=0表示为成员数
static int libxml_get_member_size(xmlDocPtr doc, const char *xpath) {
	xmlXPathObjectPtr xpathObj = NULL;
	xmlXPathContextPtr ctx = NULL;

	if (create_xpath(doc, xpath, &ctx, &xpathObj,NULL,0) != 0) {
		return -1;
	}
	//
	int size = xpathObj->nodesetval ? xpathObj->nodesetval->nodeNr:0;
	xmlXPathFreeObject(xpathObj);
	xmlXPathFreeContext(ctx);

	return size;
}








/********************************** sg_xml **********************************/
typedef struct {
	//如果是载入文件的，总是存在的
	VTDNav *vn;
	//需要的时候才创建
	xmlDocPtr doc;
	char *buffer;
} sg_xml_doc;

#define cast_xml_doc(doc) (sg_xml_doc*)doc

static int string_to_double(const char *str, double *value) {
	//*value=xmlXPathCastStringToNumber(str);
	if (str == NULL || strlen(str) == 0) {
		return -1;
	}
	char *end = NULL;
	double d = strtod(str, &end);
	if (end - str == strlen(str)) {
		*value = d;
		return 0;
	} else {
		return -1;
	}
}

static int string_to_int(const char *str, int *value) {
	if (str == NULL || strlen(str) == 0) {
		return -1;
	}
	char *end = NULL;
	long int i = strtol(str, &end, 10);
	if (end - str == strlen(str)) {
		*value = i;
		return 0;
	} else {
		return -1;
	}
}

static int string_to_bool(const char *str, bool *value) {
	if (str == NULL || strlen(str) == 0) {
		return -1;
	}
	int error = 0;
	if (strcmp(str, "true") == 0) {
		*value = TRUE;
	} else if (strcmp(str, "false") == 0) {
		*value = FALSE;
	} else {
		error = -1;
	}
	return error;
}

static char *double_to_string(double *d) {
	char *buf = NULL;
	int size = asprintf(&buf, "%f", *d);
	if (size > 0) {
		return buf;
	} else {
		free(buf);
		return NULL;
	}
}

static char *int_to_string(int *i) {

	char *buf = NULL;
	int size = asprintf(&buf, "%d", *i);
	if (size > 0) {
		return buf;
	} else {
		free(buf);
		return NULL;
	}
}

static char *bool_to_string(bool *b) {
	if (*b == TRUE) {
		return strdup("true");
	} else {
		return strdup("false");
	}
}
typedef char*(*to_string_fn)(void *);
typedef int (*to_value_fn)(const char *, void *value);

static char * read_file(const char *path) {
	struct stat s;
	if (stat(path, &s) != 0) {
		return NULL;
	}
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		return NULL;
	}
	int size = s.st_size;
	char *buffer = malloc(size + 1);
	if (buffer == NULL || fread(buffer, 1, size, file) != size) {
		fclose(file);
		free(buffer);
		return NULL;
	}
	buffer[size] = '\0';
	fclose(file);
	return buffer;
}

static sg_xml_doc_t *create_xml_doc(const char *buffer, int size,
bool isFreeBuffer) {
	sg_xml_doc *xml = malloc(sizeof(sg_xml_doc));
	if (xml == NULL) {
		return NULL;
	}
	memset(xml, 0, sizeof(sg_xml_doc));
	if (buffer == NULL || size == 0) {
		//表示为创建空白的
		xml->doc = libxml_create_doc(NULL, 0);
	} else {
		xml->vn = vtdxml_create_doc(buffer, size);
		//true表示需要释放这个buffer
		if (isFreeBuffer) {
			xml->buffer = buffer;
		}

		//开始的时候还不需要创建libxml的对象
		xml->doc = NULL;
	}
	return xml;
}
static int _xml_get_value(sg_xml_doc_t *doc, const char *xpath, void *valuePtr,
		to_value_fn fn) {
	sg_xml_doc *xml = cast_xml_doc(doc);
	char *str = NULL;
	int error = -1;

	if (xml->doc != NULL) {
		//需要同步到vtd-xml
		char *buffer = NULL;
		if (libxml_doc_to_str(xml->doc, &buffer) != 0) {
			//失败
			return -1;
		}
		VTDNav * newVN = vtdxml_create_doc(buffer, strlen(buffer));

		if (newVN == NULL) {
			//创建vtd-xml失败，释放buffer
			free(buffer);
			return -1;
		}
		//创建成功，释放libxml
		libxml_destroy_doc(xml->doc);
		xml->doc = NULL;

		//这2个应该是不存在的了，这里只是说明不要忘记释放
		vtdxml_destroy_doc(xml->vn);
		free(xml->buffer);
		//
		xml->vn = newVN;
		xml->buffer = buffer;

	}

	error = vtdxml_get_string(xml->vn, xpath, &str);

	if (error != 0) {
		return error;
	}

	if (fn != NULL) {
		error = fn(str, valuePtr);
		free(str);
	} else {
		//void *value => char **value
		*((char **) valuePtr) = str;
		//不能够释放str
		error = 0;
	}

	return error;
}

static int _xml_set_value(sg_xml_doc_t *doc, const char *xpath, void *valuePtr,
		to_string_fn fn) {
	sg_xml_doc *xml = cast_xml_doc(doc);
	char *temp = NULL;
	char *str = NULL;
	if (fn != NULL) {
		temp = fn(valuePtr);
		str = temp;
	} else {
		char **p = valuePtr;
		str = *p;
	}
	int error = -1;

	if (xml->doc != NULL) {
		//如果存在了libxml，就对其操作
		error = libxml_set_string(xml->doc, xpath, str);
	} else {
		error = vtdxml_set_string(xml->vn, xpath, str);
		if (error != 0) {
			//设置失败，就需要创建libxml
			xml->doc = libxml_create_doc(xml->vn->XMLDoc + xml->vn->docOffset,
					xml->vn->docLen);
			if (xml->doc != NULL) {
				//vtd-xml就无效了，马上可以释放了
				vtdxml_destroy_doc(xml->vn);
				free(xml->buffer);

				xml->vn = NULL;
				xml->buffer = NULL;

				//创建成功
				error = libxml_set_string(xml->doc, xpath, str);
			}
		}

	}
	free(temp);
	return error;

}












void sg_xml_init(void)
{
	libxml_init();
}
void sg_xml_free(void)
{
	libxml_free();
}

sg_xml_doc_t *sg_xml_alloc_doc(const char *path)
{
	char *buffer = read_file(path);
	if (buffer == NULL) {
		return NULL;
	}
	return create_xml_doc(buffer, strlen(buffer), TRUE);
}

sg_xml_doc_t *sg_xml_alloc_doc2(const char *buffer, size_t size)
{
	return create_xml_doc(buffer, size, FALSE);
}

sg_xml_doc_t *sg_xml_alloc_doc3(void) {
	return create_xml_doc(NULL, 0, FALSE);
}

/* 不要忘记使用xml_free_string释放xml_str_t */
int sg_xml_doc_to_str(sg_xml_doc_t *doc, sg_xml_str_t **str) {
	sg_xml_doc *xml = cast_xml_doc(doc);
	if (xml->doc != NULL) {
		return libxml_doc_to_str(xml->doc, str);
	} else {
		return vtdxml_doc_to_str(xml->vn, str);
	}

}

int sg_xml_get_member_size(sg_xml_doc_t *doc, const char *xpath) {
	sg_xml_doc *xml = cast_xml_doc(doc);
	if (xml->doc != NULL) {
		return libxml_get_member_size(xml->doc, xpath);
	} else {
		return vtdxml_get_member_size(xml->vn, xpath);
	}

}

enum xml_val_type sg_xml_get_type(sg_xml_doc_t *doc, const char *xpath)
{
	sg_xml_doc *xml = cast_xml_doc(doc);
	char *str = NULL;
	if (sg_xml_get_string(doc, xpath, &str) != 0) {
		return SGXMLVALTYPE_ERROR;
	}

	bool b = TRUE;
	double d = 0;
	int i = 0;
	if (strlen(str) == 0) {
		return SGXMLVALTYPE_NULL;
	} else if (string_to_bool(str, &b) == 0) {
		return SGXMLVALTYPE_BOOL;
	} else if (string_to_double(str, &d) == 0 || string_to_int(str, &i)) {
		return SGXMLVALTYPE_NUMBER;
	} else {
		return SGXMLVALTYPE_STRING;
	}

}

int sg_xml_get_int(sg_xml_doc_t *doc, const char *xpath, int *value)
{
	return _xml_get_value(doc, xpath, value, string_to_int);
}

int sg_xml_get_double(sg_xml_doc_t *doc, const char *xpath, double *value)
{
	return _xml_get_value(doc, xpath, value, string_to_double);
}

int sg_xml_get_bool(sg_xml_doc_t *doc, const char *xpath, bool *value)
{
	return _xml_get_value(doc, xpath, value, string_to_bool);
}

/* 不要忘记使用xml_free_string释放xml_str_t */
int sg_xml_get_string(sg_xml_doc_t *doc, const char *xpath, sg_xml_str_t **value)
{
	return _xml_get_value(doc, xpath, value, NULL);
}

int sg_xml_set_int(sg_xml_doc_t *doc, const char *xpath, int value)
{
	return _xml_set_value(doc, xpath, &value, int_to_string);
}

int sg_xml_set_double(sg_xml_doc_t *doc, const char *xpath, double value)
{
	return _xml_set_value(doc, xpath, &value, double_to_string);
}

int sg_xml_set_bool(sg_xml_doc_t *doc, const char *xpath, bool value)
{
	return _xml_set_value(doc, xpath, &value, bool_to_string);
}

int sg_xml_set_string(sg_xml_doc_t *doc, const char *xpath, const char *value)
{
	return _xml_set_value(doc, xpath, &value, NULL);
}

void sg_xml_free_string(sg_xml_str_t *str)
{
	free(str);
}

void sg_xml_free_doc(sg_xml_doc_t *doc)
{
	sg_xml_doc *xml = cast_xml_doc(doc);
	vtdxml_destroy_doc(xml->vn);
	libxml_destroy_doc(xml->doc);
	free(xml->buffer);
	free(xml);
}