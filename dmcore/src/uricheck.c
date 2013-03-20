/*
 * libdmclient
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * David Navarro <david.navarro@intel.com>
 *
 */

/*!
 * @file <uricheck.c>
 *
 * @brief uri syntax validation functions
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "error_macros.h"
#include "log.h"

#include "internals.h"

#define OMADM_TOKEN_PROP "?prop="
#define OMADM_TOKEN_LIST "?list="

/******
 * A valid URI for OMADM is :
 *    uri        = node_uri[ property | list ]
 *    property   = "?prop=" prop_name
 *    list       = "?list=" attribute
 *    node_uri   = "." | [ "./" ] path
 *    path       = segment *( "/" segment )
 *    segment    = *( pchar | "." ) pchar
 *    pchar      = unreserved | escaped | ":" | "@" | "&" | "=" | "+" | "$" | ","
 *    unreserved = alphanum | mark
 *    mark       = "-" | "_" | "!" | "~" | "*" | "'" | "(" | ")"
 *    escaped    = "%" hex hex
 *    hex        = digit | "A" | "B" | "C" | "D" | "E" | "F" |
 *                         "a" | "b" | "c" | "d" | "e" | "f"
 *    alphanum   = alpha | digit
 *    alpha      = lowalpha | upalpha
 *    lowalpha   = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
 *                 "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
 *                 "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
 *    upalpha    = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
 *                 "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
 *                 "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
 *    digit      = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
 *                 "8" | "9"
 *
 *****/

static bool prv_check_hex(char target)
{
    if (((target >= 'A') && (target <= 'F'))
     || ((target >= 'a') && (target <= 'f'))
     || ((target >= '0') && (target <= '9')))
    {
        return true;
    }
    return false;
}

static bool prv_check_pchar(char target)
{
    if (((target >= 'A') && (target <= 'Z'))
     || ((target >= 'a') && (target <= 'z'))
     || ((target >= '0') && (target <= '9')))
    {
        return true;
    }
    switch (target)
    {
    case ':':
    case '@':
    case '&':
    case '=':
    case '+':
    case '$':
    case ',':
    case '-':
    case '_':
    case '!':
    case '~':
    case '*':
    case '\'':
    case '(':
    case ')':
        return true;
    default:
        break;
    }

    return false;
}

int uri_validate_path(char * path_str,
                      const uint16_t max_depth,
                      const uint16_t max_len)
{
    int len = 0;

    if (*path_str == 0)
    {
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }
    while (*path_str != 0 && *path_str != '/')
    {
        switch (*path_str)
        {
        case '.':
            if (*(path_str+1) == '/')
            {
                return OMADM_SYNCML_ERROR_COMMAND_FAILED;
            }
            path_str++;
            len++;
            break;
        case '%':
            if (!prv_check_hex(*(path_str+1))
             && !prv_check_hex(*(path_str+2)))
            {
                return OMADM_SYNCML_ERROR_COMMAND_FAILED;
            }
            path_str += 3;
            len += 3;
            break;
         default:
            if (!prv_check_pchar(*path_str))
            {
                return OMADM_SYNCML_ERROR_COMMAND_FAILED;
            }
            path_str++;
            len++;
            break;
        }
        if (max_len && len > max_len)
        {
            return OMADM_SYNCML_ERROR_URI_TOO_LONG;
        }
    }

    if (0 == len)
    {
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }

    if (*path_str == '/')
    {
        switch (max_depth)
        {
        case 0:
            // unlimited depth
            return uri_validate_path(path_str+1, max_depth, max_len);
        case 1:
            // we already parsed one segment and there is more
            return OMADM_SYNCML_ERROR_URI_TOO_LONG;
        default:
            return uri_validate_path(path_str+1, max_depth-1, max_len);
        }
    }

    return OMADM_SYNCML_ERROR_NONE;
}

int uri_validate(const uint16_t max_total_len,
                 const uint16_t max_depth,
                 const uint16_t max_segment_len,
                 const char * uri,
                 char ** oNodeURI,
                 char ** oPropId)
{
    DMC_ERR_MANAGE;

    unsigned int uri_len;
    char * node_uri = NULL;
    char * prop_str;
    char * path_str;

    DMC_FAIL_ERR(!uri, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    DMC_FAIL_ERR(oPropId && !oNodeURI, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    uri_len = strlen(uri);
    DMC_FAIL_ERR(uri_len == 0, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    if (max_total_len)
    {
        DMC_FAIL_ERR(uri_len > max_total_len, OMADM_SYNCML_ERROR_URI_TOO_LONG);
    }

    DMC_FAIL_ERR(strstr(uri, OMADM_TOKEN_LIST) != NULL, OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);

    DMC_FAIL_NULL(node_uri, strdup(uri), OMADM_SYNCML_ERROR_DEVICE_FULL);

    prop_str = strstr(node_uri, OMADM_TOKEN_PROP);
    if (prop_str)
    {
        DMC_FAIL_ERR(!oPropId, OMADM_SYNCML_ERROR_NOT_ALLOWED);
        DMC_FAIL_NULL(*oPropId, strdup(prop_str + strlen(OMADM_TOKEN_PROP)), OMADM_SYNCML_ERROR_DEVICE_FULL);
        *prop_str = 0;
        uri_len = strlen(node_uri);
        DMC_FAIL_ERR(uri_len == 0, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    }

    DMC_FAIL_ERR((node_uri)[uri_len-1] == '/', OMADM_SYNCML_ERROR_COMMAND_FAILED);

    path_str = node_uri;

    if ((node_uri)[0] == '.')
    {
        switch ((node_uri)[1])
        {
        case 0:
            path_str = NULL;
            break;
        case '/':
            path_str += 2;
            break;
        default:
            break;
        }
    }
    else
    {
        node_uri = str_cat_2("./", path_str);
        if (!node_uri)
        {
            node_uri = path_str;
            DMC_FAIL(OMADM_SYNCML_ERROR_DEVICE_FULL);
        }
        free(path_str);
        path_str = node_uri + 2;
    }

    if (path_str)
    {
        DMC_FAIL(uri_validate_path(path_str, max_depth, max_segment_len));
    }

    if (oNodeURI)
    {
        *oNodeURI = node_uri;
    }
    else
    {
        free(node_uri);
    }

    DMC_LOGF("%s exitted with error %d",__FUNCTION__, DMC_ERR);
    return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

    if (node_uri)
    {
        free(node_uri);
    }
    if (oPropId && *oPropId)
    {
        free(*oPropId);
        *oPropId = NULL;
    }
    DMC_LOGF("%s exitted with error %d",__FUNCTION__, DMC_ERR);

    return DMC_ERR;
}
