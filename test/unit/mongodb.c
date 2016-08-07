#include <stdio.h>

#include "mongo.h"

int main()
{
	int times=2, port=27017, msec=10000;
	sg_mongo_cli *client;
	sg_mongo_coll *coll;
	char *ip = "172.20.220.156";
	const char *db_name = "gcctest";
	const char *coll_name = "youtube";
	char *jsonstr = "{\"Youtube\":{\"URL\": \"https://www.youtube.com/watch?v=uBJvetXSSPg\",\"Like\": 112,\"Share\": 200}}";
	char *query = "{\"Youtube.URL\":\"https://www.youtube.com/watch?v=uBJvetXSSPg\"}";		
	char *update = "{\"$set\":{\"Youtube\":{\"URL\": \"https://www.youtube.com/watch?v=uBJvetXSSPg\",\"Like\": 1120,\"Share\": 2000}}}";
	/* "$set" is a precondition like set in SQL update, the following json string is the update to perform */
	
	/* initialize libmongoc's internals() */
	sg_mongo_init();

	/* try to reconnect once fail to connect at first time */
	if (sg_mongo_connect_withtimeout(ip, port, msec, &client) != 0)  
		if (sg_mongo_reconnect(ip, port, msec, times, &client) != 0) {
			sg_mongo_quit();	/* MongoDB connection error,clear_my_client()has been internally called */
			return -1;
		}
	/* try to access to the specified collection */
	if (sg_mongo_connect_collection(client, &coll, db_name, coll_name) != 0) { 
		sg_mongo_client_disconnect(client);
		sg_mongo_quit();
		return -1;
	}
	if (sg_mongo_collection_insert_easy(coll, jsonstr) != 0) 
		return -1;
	else
		printf("Insert data successfully\n");

	if (sg_mongo_collection_update_easy(coll, query, update)) 
		return -1; 
	else
		printf("update data successfully\n");
	
	sg_mongo_collection_disconnect(coll);
	sg_mongo_client_disconnect(client);
	sg_mongo_quit();
	return 0;	
}
