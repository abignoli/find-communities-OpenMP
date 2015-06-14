#include "execution-settings.h"

char * file_format_name(int id){
	switch(id) {
	case FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED:
		return FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED_NAME;
	case FILE_FORMAT_EDGE_LIST_WEIGHTED:
		return FILE_FORMAT_EDGE_LIST_WEIGHTED_NAME;
	case FILE_FORMAT_METIS:
		return FILE_FORMAT_METIS_NAME;
	default:
		return FILE_FORMAT_INVALID_NAME;
	}
}
