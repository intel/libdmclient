/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2006 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file <dyn_buf.c>
 *
 * @brief Main source file for implementing a dynamic buffer.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "error.h"
#include "dyn_buf.h"

void dmc_buf_make(dmc_buf *buffer, unsigned int block_size)
{
	memset(buffer, 0, sizeof(*buffer));
	buffer->block_size = block_size;
}

void dmc_buf_free(dmc_buf *buffer)
{
	if (buffer->buffer) {
		free(buffer->buffer);
		buffer->buffer = NULL;
	}
}

int dmc_buf_append(dmc_buf *buffer, const uint8_t *data, unsigned int data_size)
{
	int ret_val = DMC_ERR_NONE;
	uint8_t *block = NULL;
	unsigned int new_size = data_size + buffer->size;
	unsigned int new_max_size = 0;

	if (new_size > buffer->max_size) {
		new_max_size = ((new_size / buffer->block_size) + 1) *
				buffer->block_size;

		block = realloc(buffer->buffer, new_max_size);
		if (block) {
			buffer->buffer = block;
			buffer->max_size = new_max_size;
		} else
			ret_val = DMC_ERR_OOM;
	}

	if (ret_val == DMC_ERR_NONE) {
		memcpy(&buffer->buffer[buffer->size], data, data_size);
		buffer->size = new_size;
	}

	return ret_val;
}

int dmc_buf_append_str(dmc_buf *buffer, const char *data)
{
	int slen = strlen(data);

	if (slen == 0)
		return DMC_ERR_NONE;
	else
		return dmc_buf_append(buffer, (const uint8_t *) data, slen);
}

int dmc_buf_zero_terminate(dmc_buf *buffer)
{
	uint8_t data[1];

	data[0] = 0;

	return dmc_buf_append(buffer, data, 1);
}

uint8_t *dmc_buf_adopt(dmc_buf *buffer)
{
	uint8_t *ret_val = buffer->buffer;

	memset(buffer, 0, sizeof(dmc_buf));

	return ret_val;
}

void dmc_ptr_array_make(dmc_ptr_array *array, unsigned int block_size,
					dmc_ptr_array_des destructor)
{
	array->size = array->max_size = 0;
	array->block_size = block_size;
	array->destructor = destructor;
	array->array = NULL;
}

void dmc_ptr_array_make_from(dmc_ptr_array *array, void **new_array,
				unsigned int size, unsigned int block_size,
				dmc_ptr_array_des destructor)
{
	array->size = array->max_size = size;
	array->block_size = block_size;
	array->destructor = destructor;
	array->array = new_array;
}

void dmc_ptr_array_adopt(dmc_ptr_array *array, void **carray,
				unsigned int *size)
{
	*carray = array->array;
	*size = array->size;
	memset(array, 0, sizeof(*array));
}

void dmc_ptr_array_free(dmc_ptr_array *array)
{
	unsigned int i = 0;

	if (array->array == NULL)
		return;

	if (array->destructor)
		for (; i < array->size; ++i)
			if (array->array[i])
				array->destructor(array->array[i]);

	free(array->array);
	array->array = NULL;
}

void dmc_ptr_array_free_callback(void *array)
{
	if (array)
	{
		dmc_ptr_array_free((dmc_ptr_array *) array);
		free(array);
	}
}

int dmc_ptr_array_append(dmc_ptr_array *array, void *pointer)
{
	void *buffer = NULL;
	int ret_val = DMC_ERR_NONE;
	unsigned int new_max_size = 0;

	if (array->size == array->max_size) {
		new_max_size = array->size + array->block_size;

		buffer = realloc(array->array, new_max_size * sizeof(void *));
		if (buffer) {
			array->array = buffer;
			array->max_size = new_max_size;
		} else
			ret_val = DMC_ERR_OOM;
	}

	if (ret_val == DMC_ERR_NONE) {
		array->array[array->size] = pointer;
		++array->size;
	}

	return ret_val;
}

void dmc_ptr_array_delete(dmc_ptr_array *array, unsigned int index)
{
	if (index < array->size) {
		if (array->destructor && array->array[index])
			array->destructor(array->array[index]);

		for (; index < array->size - 1; ++index)
			array->array[index] = array->array[index + 1];

		--array->size;
	}
}
