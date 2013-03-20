/*
 * libdmclient test materials
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <syncml_error.h>

#include "memory_mo_util.h"

static void prv_free_node(memory_node_t *node)
{
    memory_node_t * child;

    if (NULL == node)
    {
        return;
    }
    child = node->children;
    
    while(child)
    {
        child->parent = NULL;
        prv_free_node(child);
        child = child->next;
    }
    if (node->parent)
    {
        child = node->parent->children;
        if (child == node)
        {
            node->parent->children = node->next;
        }
        else
        {
            while (child && child->next != node)
                child = child->next;
            if (child)
                child->next = node->next;
        }
    }
    
    dmtree_node_free(node->content);
    if (node->name) free(node->name);
    if (node->acl) free(node->acl);
    
    free(node);
}

static memory_node_t * prv_find_subnode(memory_node_t *node,
                                        const char *iURI,
                                        bool exact)
{
    char * nextName;
    char * name;
    memory_node_t * result;
    memory_node_t * child;

    nextName = strchr(iURI, '/');
    if (nextName)
    {
        name = (char*)malloc(nextName - iURI + 1);
        if (NULL == name) return NULL;
        strncpy(name, iURI, nextName - iURI);
        name[nextName - iURI] = 0;
        nextName++;
    }
    else
    {
        name = (char *)iURI;
    }

    child = node->children;
    while(child && strcmp(child->name, name))
    {
        child = child->next;
    }

    if (NULL != child)
    {
        if (NULL != nextName)
        {
            result = prv_find_subnode(child, nextName, exact);
        }
        else
        {
            result = child;
        }
    }
    else
    {
        if (false == exact)
        {
            result = node;
        }
        else
        {
            result = NULL;
        }
    }

    if (NULL != nextName) free(name);
    return result;
}

static memory_node_t * prv_find_node(memory_node_t *root,
                                     const char *iURI,
                                     bool exact)
{
    size_t root_len;
    char * uri;
    char * name;
    memory_node_t * child;
        
    if (!strcmp(iURI, root->name))
        return root;
        
    root_len = strlen(root->name);
    if (strlen(iURI) < root_len)
        return NULL;
    if (strncmp(root->name, iURI, root_len))
        return NULL;
    
    uri = strdup(iURI + root_len);
    if (!uri || *uri != '/')
        return NULL;
    name = uri + 1;

    child = prv_find_subnode(root, name, exact);
    
    free(uri);
    return child;
}

static memory_node_t * prv_create_node(memory_node_t * parent,
                                       char * name)
{
    memory_node_t * newNode;
    
    if (!parent || !name)
        return NULL;
        
    newNode = (memory_node_t *)malloc(sizeof(memory_node_t));
    if (NULL == newNode) return NULL;
    
    memset(newNode, 0, sizeof(memory_node_t));

    newNode->name = strdup(name);
    if (NULL == newNode->name) goto on_error;

    newNode->uri = (char *)malloc(strlen(parent->uri) + strlen(newNode->name) + 2);
    if (NULL == newNode->uri) goto on_error;
    sprintf(newNode->uri, "%s/", parent->uri);
    strcat(newNode->uri, newNode->name);
    
    newNode->type = OMADM_NODE_IS_INTERIOR;
    newNode->next = parent->children;
    parent->children = newNode;
    newNode->parent = parent;

    return newNode;
    
on_error:
    if (newNode->name) free(newNode->name);
    if (newNode->uri) free(newNode->uri);
    free(newNode);
    return NULL;
}

int memory_mo_init(char * iURI,
                   char * iACL,
                   void **oData)
{
    memory_node_t * root;
    
    if (!iURI || !iACL || !oData)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    root = (memory_node_t *)malloc(sizeof(memory_node_t));
    if (!root) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    
    memset(root, 0, sizeof(memory_node_t));
    
    root->type = OMADM_NODE_IS_INTERIOR;
    root->name = strdup(iURI);
    if (NULL == root->name)
    {
        free(root);
        return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }
    root->uri = strdup(iURI);
    if (NULL == root->uri)
    {
        free(root->name);
        free(root);
        return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }
    root->acl = strdup(iACL);
    if (NULL == root->acl)
    {
        free(root->name);
        free(root->uri);
        free(root);
        return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }
    
    *oData = (void *)root;
    
    return OMADM_SYNCML_ERROR_NONE;
}

void memory_mo_close(void * iData)
{
    prv_free_node((memory_node_t *)iData);
}

int memory_mo_is_node(const char *iURI,
                      omadmtree_node_kind_t *oNodeType,
                      void *iData)
{
    memory_node_t * root = (memory_node_t *)iData;
    memory_node_t * target;

    if (!root) return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    target = prv_find_node(root, iURI, true);
    if (target)
    {
        *oNodeType = target->type;
    }
    else
    {
        *oNodeType = OMADM_NODE_NOT_EXIST;
    }

    return OMADM_SYNCML_ERROR_NONE;
}

int memory_mo_get(dmtree_node_t *nodeP,
                  void *iData)
{
    int err = OMADM_SYNCML_ERROR_NONE;
    memory_node_t * root = (memory_node_t *)iData;
    memory_node_t * target;

    if (!nodeP || !nodeP->uri || !root)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    nodeP->format = NULL;
    nodeP->type = NULL;
    nodeP->data_size = 0;
    nodeP->data_buffer = NULL;

    target = prv_find_node(root, nodeP->uri, true);
    if (NULL == target) return OMADM_SYNCML_ERROR_NOT_FOUND;
    
    switch (target->type)
    {
    case OMADM_NODE_IS_INTERIOR:
        nodeP->format = strdup("node");
        if (!nodeP->format)
        {
            err = OMADM_SYNCML_ERROR_DEVICE_FULL;
        }
        else
        {
            char * string;
            if (target->children)
            {
                memory_node_t * child = target->children->next;                
                string = strdup(target->children->name);
                if (!string) err = OMADM_SYNCML_ERROR_DEVICE_FULL;
                while (child && OMADM_SYNCML_ERROR_NONE == err)
                {
                    char * tmp = (char *)malloc(strlen(string) + strlen(child->name) + 2);
                    if (tmp)
                    {
                        sprintf(tmp, "%s/", string);
                        strcat(tmp, child->name);
                        free(string);
                        string = tmp;
                    }
                    else
                    {
                        err = OMADM_SYNCML_ERROR_DEVICE_FULL;
                    }
                    child = child->next;
                }
                if (OMADM_SYNCML_ERROR_NONE == err)
                {
                    nodeP->data_buffer = string;
                    nodeP->data_size = strlen(string) + 1;
                }
                else if (string)
                {
                    free(string);
                }
            }
        }
        break;
    case OMADM_NODE_IS_LEAF:
        if (NULL == dmtree_node_copy(nodeP, target->content))
        {
            err = OMADM_SYNCML_ERROR_DEVICE_FULL;
        }
        break;
    default:
        err = OMADM_SYNCML_ERROR_NOT_FOUND;
    }

    return err;
}

int memory_mo_getACL(const char *iURI,
                     char **oValue,
                     void *iData)
{
    int err = OMADM_SYNCML_ERROR_NOT_FOUND;
    memory_node_t * root = (memory_node_t *)iData;
    memory_node_t * target;

    if (!root || !oValue || !iURI)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    
    target = prv_find_node(root, iURI, true);
    if (target)
    {
        *oValue = NULL;

        if (target->acl)
        {
            *oValue = strdup(target->acl);
            if (!*oValue) err = OMADM_SYNCML_ERROR_DEVICE_FULL;
        }
        err = OMADM_SYNCML_ERROR_NONE;
    }

    return err;
}

int memory_mo_set(const dmtree_node_t *nodeP,
                  void *iData)
{
    int err = OMADM_SYNCML_ERROR_NOT_FOUND;
    memory_node_t * root = (memory_node_t *)iData;
    memory_node_t * target;

    if (!root || !nodeP)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    
    target = prv_find_node(root, nodeP->uri, true);
    if (target)
    {
        err = OMADM_SYNCML_ERROR_NONE;
        if (!strcmp(target->content->uri, nodeP->uri))
        {
            dmtree_node_clean(target->content, true);
            if (NULL == dmtree_node_copy(target->content,
                                         nodeP))
            {
                err = OMADM_SYNCML_ERROR_DEVICE_FULL;
            }
        }
    }
    else
    {
        target = prv_find_node(root, nodeP->uri, false);
        if (target)
        {
            char * curNodeName;
            memory_node_t * newNode = NULL;

            err = OMADM_SYNCML_ERROR_DEVICE_FULL;
            curNodeName = strdup(nodeP->uri + strlen(target->uri) + 1);
            while (NULL != curNodeName && 0 != *curNodeName)
            {
                char * nextNode = strchr(curNodeName, '/');
                if (nextNode)
                {
                    *nextNode = 0;
                    nextNode++;
                }
                newNode = prv_create_node(target, curNodeName);
                if (newNode)
                {
                    target = newNode;
                }
                else
                {
                    // break equivalent
                     nextNode = NULL;
                }
                curNodeName = nextNode;
            }
            if (NULL != newNode)
            {
                if (strcmp(nodeP->format, "node"))
                {
                    newNode->type = OMADM_NODE_IS_LEAF;
                }
                newNode->content = dmtree_node_dup(nodeP);
                if (NULL == newNode->content)
                {
                    prv_free_node(newNode);
                }
                else
                {
                    err = OMADM_SYNCML_ERROR_NONE;
                }
            }
        }
    }
    
    return err;
}

int memory_mo_setACL(const char *iURI,
                     const char *iACL,
                     void *iData)
{
    int err = OMADM_SYNCML_ERROR_NOT_FOUND;
    memory_node_t * root = (memory_node_t *)iData;
    memory_node_t * target;

    if (!root || !iURI)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    target = prv_find_node(root, iURI, true);
    if (target)
    {
        char * tmp = NULL;
        
        err = OMADM_SYNCML_ERROR_NONE;        
        if (iACL)
        {
            tmp = strdup(iACL);
            if (!tmp)
            {
                err = OMADM_SYNCML_ERROR_DEVICE_FULL;
            }
        }
        if (OMADM_SYNCML_ERROR_NONE == err)
        {
            if (target->acl)
            {
                free(target->acl);
            }
            target->acl = tmp;
        }
    }

    return err;
}

int memory_mo_delete(const char *iURI,
                     void *iData)
{
    int err = OMADM_SYNCML_ERROR_NOT_FOUND;
    memory_node_t * root = (memory_node_t *)iData;
    memory_node_t * target;

    if (!root || !iURI)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    target = prv_find_node(root, iURI, true);
    if (target)
    {
        err = OMADM_SYNCML_ERROR_NONE;
        prv_free_node(target);
    }

    return err;
}

int memory_mo_findURN(const char *iURN,
                      char ***oURL,
                      void *iData)
{
    return OMADM_SYNCML_ERROR_NOT_FOUND;
}

