/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef SILENT_SWITCH_H
#define SILENT_SWITCH_H

/*
 * #define SILENT
 * Makes all files containing:
 *
 * #include "execution-parameters.h"
 * #ifdef SILENT_SWITCH_ON
 * #define printf(...)
 * #endif
 *
 * unable to use printf. (i.e. this make them silent)
 */
#define SILENT_SWITCH_ON

//#define SILENT_SWITCH_COMMUNITY_COMPUTATION_ON

#define SILENT_SWITCH_ON_COMMUNITY_DEVELOPMENT

#define SILENT_SWITCH_DYNAMIC_GRAPH_ON

#define SILENT_SWITCH_SORT_ON


#endif
