#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cjson/cJSON.h"
#include "json.h"

static cJSON * cast_to_cJSON(void *doc) {
	return doc;
}
//获得一个key，返回NULL表示不能够获得，否则需要释放返回的key，同时会更新offset的值
//https://tools.ietf.org/html/rfc6901
static char * get_key(const char *json_pointer, int *offset) {
	int len = strlen(json_pointer);
	if (*offset >= len) {
		return NULL;
	}
	if (json_pointer[*offset] != '/') {
		return NULL;
	}
	char key[len - *offset];
	memset(key, 0, sizeof(key));
	int j = 0;
	int i = 0;
	for (i = *offset + 1; i < len;) {
		char c = json_pointer[i];
		if (c == '~') {
			//~1 => /
			//~0 => ~
			if (i + 1 < len) {
				char c2 = json_pointer[i + 1];
				if (c2 == '1') {
					key[j++] = '/';
				} else if (c2 == '0') {
					key[j++] = '~';
				} else {
					key[j++] = c;
					key[j++] = c2;
				}
				i += 2;
			} else {
				//没有了
				key[j++] = c;
				i++;
			}
		} else if (c != '/') {
			key[j++] = c;
			i++;
		} else {
			break;
		}
	}

	char * key2 = malloc(j + 1);
	if (key2 == NULL) {
		return NULL;
	}
	*offset = i;
	strncpy(key2, key, j);
	key2[j] = '\0';

	//printf("key=%s,offset=%d\n", key2, *offset);
	return key2;
}
static cJSON * get_cJSON(cJSON *parent, const char *json_pointer, int offset) {
	if (parent == NULL || parent->type == cJSON_NULL) {
		return NULL;
	}
	if (offset == strlen(json_pointer)) {
		//如果为"" => obj
		return parent;
	}
	// / => ""  => obj[""]
	// /key => "key" =>obj["key"]
	char * key = get_key(json_pointer, &offset);

	if (key == NULL) {
		return NULL;
	}
	cJSON *value = NULL;
	if (parent->type == cJSON_Array) {
		//key必须为0 or 123,456
		int index = atoi(key);
		//strtol()
		value = cJSON_GetArrayItem(parent, index);
	} else if (parent->type == cJSON_Object) {
		value = cJSON_GetObjectItem(parent, key);
	} else {
		//null,number,string,boolean
		//不应该有key了
		value = NULL;
	}

	free(key);

	if (value != NULL && value->type != cJSON_NULL) {

		return get_cJSON(value, json_pointer, offset);
	} else {
		return NULL;
	}

}
static cJSON *get_value(cJSON *root, const char *json_pointer) {
	return get_cJSON(root, json_pointer, 0);
}
static int set_value(cJSON *root, const char *json_pointer, cJSON *value) {
	if (root == NULL) {
		return -1;
	}
	const int len = strlen(json_pointer);
	int offset = 0;
	if (offset == len) {
		//如果为""
		return -1;
	}
	cJSON *parent = root;
	char *key = NULL;
	while (1) {
		free(key);
		key = get_key(json_pointer, &offset);
		if (key == NULL) {
			return -1;
		}
		if (offset >= len) {
			//表示没有下一个key了
			break;
		}

		cJSON *child = NULL;
		if (parent->type == cJSON_Object) {
			child = cJSON_GetObjectItem(parent, key);
			if (child == NULL) {
				//总是创建object而不是数组
				child = cJSON_CreateObject();
				cJSON_AddItemToObject(parent, key, child);
			} else if (child->type != cJSON_Object && child->type != cJSON_Array) {
				child = cJSON_CreateObject();
				cJSON_ReplaceItemInObject(parent, key, child);
			} else {
				//do nothing
			}
		} else if (parent->type == cJSON_Array) {
			int index = atoi(key);
			child = cJSON_GetArrayItem(parent, index);
			if (child == NULL) {
				//总是创建object而不是数组
				child = cJSON_CreateObject();
				cJSON_InsertItemInArray(parent, index, child);
			} else if (child->type != cJSON_Object && child->type != cJSON_Array) {
				child = cJSON_CreateObject();
				cJSON_ReplaceItemInArray(parent, index, child);
			} else {
				//do nothing
			}
		} else {
			//不应该执行到这里
			printf("===>not impossible\n");

		}

		parent = child;

	}

	if (parent->type == cJSON_Array) {
		//key必须为0 or 123,456, '-'表示添加到最后面
		if (strcmp("-", key) == 0) {
			cJSON_AddItemToArray(parent, value);
		} else {
			int index = atoi(key);
			//strtol()
			cJSON_InsertItemInArray(parent, index, value);
		}

	} else if (parent->type == cJSON_Object) {
		//cJSON_AddItemToObject(parent,key,value);
		cJSON *oldValue = cJSON_GetObjectItem(parent, key);
		if (oldValue == NULL) {
			cJSON_AddItemToObject(parent, key, value);

		} else {
			cJSON_ReplaceItemInObject(parent, key, value);

		}

		//cJSON *kk = cJSON_GetObjectItem(parent, key);

	} else {
		//null,number,string,boolean
		//不能够添加
		printf("=====>noooo\n");
	}
	free(key);
	return 0;

}
static int read_file(const char *filename, char **buf, int *size) {
	FILE *f = NULL;
	long len = 0;
	char *data = NULL;

	f = fopen(filename, "rb");
	if (f == NULL) {
		return -1;
	}
	if (fseek(f, 0, SEEK_END) != 0) {
		fclose(f);
		return -1;
	}
	len = ftell(f);
	if (len == -1) {
		fclose(f);
		return -1;
	}
	if (fseek(f, 0, SEEK_SET) != 0) {
		fclose(f);
		return -1;
	}
	data = (char*) malloc(len + 1);
	if (data == NULL) {
		fclose(f);
		return -1;
	}
	//
	if (fread(data, 1, len, f) != len) {
		free(data);
		fclose(f);
		return -1;
	}

	fclose(f);
	data[len] = '\0';
	*buf = data;
	*size = len;
	return 0;

}
json_doc_t *json_alloc_doc(const char *filename) {
	char *data = NULL;
	int size = 0;
	if (read_file(filename, &data, &size) != 0) {
		return NULL;
	}
	json_doc_t * obj = json_alloc_doc2(data, size);
	free(data);
	return obj;
}

json_doc_t *json_alloc_doc2(const void *buf, size_t size) {
	if (size==-1 || strlen(buf) == size) {
		return cJSON_Parse(buf);
	} else {
		//stack在不同的系统中的最大值不同
		//char temp[size];
		char *temp = malloc(size + 1);
		strncpy(temp, buf, size);
		temp[size] = '\0';
		cJSON *json = cJSON_Parse(temp);
		free(temp);
		return json;

	}
}

json_doc_t *json_alloc_doc3(void) {
	cJSON *json = cJSON_CreateObject();
	if (json == NULL) {
		return NULL;
	}
	return json;
}

/* 不要忘记使用json_free_string释放json_str_t */
int json_doc_to_str(json_doc_t *doc, json_str_t **str) {
	cJSON *json = cast_to_cJSON(doc);
	if (json == NULL) {
		return -1;
	}
	//*str = cJSON_Print(json);
	*str=cJSON_PrintUnformatted(json);
	return 0;
}

int json_doc_to_file(json_doc_t *doc, const char *filename) {
	cJSON *json = cast_to_cJSON(doc);
	if (json == NULL) {
		return -1;
	}

	FILE *f = fopen(filename, "wb");
	if (f == NULL) {
		return -1;
	}

	char *str = cJSON_PrintUnformatted(json);
	if (str == NULL) {
		fclose(f);
		return -1;
	}

	const int len = strlen(str);
	int j = fwrite(str, 1, len, f);
	free(str);
	fclose(f);
	return j == len ? 0 : -1;
}

int json_get_member_size(json_doc_t *doc, const char *json_pointer,int *size) {
	cJSON *json = cast_to_cJSON(doc);
	cJSON * obj = get_value(json, json_pointer);
	if (obj == NULL || obj->type != cJSON_Array) {
		return -1;
	}
	*size = cJSON_GetArraySize(obj);
	return 0;
}

enum json_val_type json_get_type(json_doc_t *doc, const char *json_pointer) {
	cJSON *json = cast_to_cJSON(doc);
	cJSON * obj = get_value(json, json_pointer);
	if (obj == NULL) {
		return JSONVALTYPE_ERROR;
	}

	int type = obj->type;
	if (type == cJSON_Object) {
		return JSONVALTYPE_OBJECT;
	} else if (type == cJSON_Array) {
		return JSONVALTYPE_ARRAY;
	} else if (type == cJSON_String) {
		return JSONVALTYPE_STRING;
	} else if (type == cJSON_Number) {
		return JSONVALTYPE_NUMBER;
	} else if (type == cJSON_True || type == cJSON_False) {
		return JSONVALTYPE_BOOL;
	} else if (type == cJSON_NULL) {
		return JSONVALTYPE_NULL;
	} else {
		return JSONVALTYPE_ERROR;
	}
}

int json_get_int(json_doc_t *doc, const char *json_pointer, int *value) {
	cJSON *json = cast_to_cJSON(doc);
	cJSON * obj = get_value(json, json_pointer);
	if (obj == NULL || obj->type != cJSON_Number) {
		return -1;
	}
	*value = obj->valueint;
	return 0;
}

int json_get_double(json_doc_t *doc, const char *json_pointer, double *value) {
	cJSON *json = cast_to_cJSON(doc);
	cJSON * obj = get_value(json, json_pointer);
	if (obj == NULL || obj->type != cJSON_Number) {
		return -1;
	}
	*value = obj->valuedouble;
	return 0;
}

int json_get_bool(json_doc_t *doc, const char *json_pointer, bool *value) {
	cJSON *json = cast_to_cJSON(doc);
	cJSON * obj = get_value(json, json_pointer);
	if (obj == NULL || (obj->type != cJSON_True && obj->type != cJSON_False)) {
		return -1;
	}
	*value = obj->valueint;
	return 0;
}

/* 不要忘记使用json_free_string释放json_str_t */
int json_get_string(json_doc_t *doc, const char *json_pointer,
		json_str_t **value) {
	cJSON *json = cast_to_cJSON(doc);
	cJSON * obj = get_value(json, json_pointer);
	if (obj == NULL || obj->type != cJSON_String) {
		return -1;
	}

	//复制一个，因为json的会自动管理
	char *temp = strdup(obj->valuestring);
	if (temp != NULL) {
		*value = temp;
		return 0;
	} else {
		return -1;
	}

}
int json_get_object(json_doc_t *doc, const char *json_pointer,
		json_doc_t **value) {
	cJSON *json = cast_to_cJSON(doc);
	cJSON * obj = get_value(json, json_pointer);
	if (obj == NULL) {
		return -1;
	}
	if (obj->type == cJSON_NULL) {
		*value = NULL;
		return 0;
	}
	if (obj->type == cJSON_Object || obj->type == cJSON_Array) {
		*value = obj;
		return 0;
	}

	return -1;
}

int json_set_object(json_doc_t *doc, const char *json_pointer,
		json_doc_t *value) {
	cJSON *json = cast_to_cJSON(doc);
	if (value == NULL) {
		value = cJSON_CreateNull();
	}
	return set_value(json, json_pointer, cast_to_cJSON(value));

}
int json_set_int(json_doc_t *doc, const char *json_pointer, int value) {
	cJSON *json = cast_to_cJSON(doc);
	return set_value(json, json_pointer, cJSON_CreateNumber(value));

}

int json_set_double(json_doc_t *doc, const char *json_pointer, double value) {
	cJSON *json = cast_to_cJSON(doc);
	return set_value(json, json_pointer, cJSON_CreateNumber(value));

}

int json_set_bool(json_doc_t *doc, const char *json_pointer, bool value) {
	cJSON *json = cast_to_cJSON(doc);
	return set_value(json, json_pointer, cJSON_CreateBool(value));
}

int json_set_string(json_doc_t *doc, const char *json_pointer,
		const char *value) {
	cJSON *json = cast_to_cJSON(doc);
	return set_value(json, json_pointer, cJSON_CreateString(value));
}

void json_free_string(json_str_t *str) {
	free(str);
}

void json_free_doc(json_doc_t *doc) {
	cJSON *json = cast_to_cJSON(doc);
	if (json != NULL) {
		cJSON_Delete(json);
	}

}

