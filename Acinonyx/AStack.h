/*
 *  AStack.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/5/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef A_STACK_H_
#define A_STACK_H_


typedef struct AStackBlock {
	vsize_t capacity, length;
	struct AStackBlock *prev;
	AObject *o[1];
} AStackBlock_t;

class AStack : public AObject {
protected:
	AStackBlock *top;
	vsize_t default_capacity;
	
public:
	AStack(vsize_t increments = 128) {
		default_capacity = increments;
		if (default_capacity < 16) default_capacity = 16;
		top = (AStackBlock_t*) malloc(sizeof(AStackBlock_t) + sizeof(AObject*) * default_capacity);
		AMEM(top);
		top->capacity = default_capacity;
		top->length = 0;
		top->prev = 0;
		OCLASS(AStack)
	}
	
	virtual ~AStack() {
		popAll();
		free(top);
	}

	virtual void push(AObject* obj) {
		if (!obj) return;
		if (top->length >= top->capacity) {
			AStackBlock_t *new_block = (AStackBlock_t*) malloc(sizeof(AStackBlock_t) + sizeof(AObject*) * default_capacity);
			AMEM(new_block);
			new_block->capacity = default_capacity;
			new_block->length = 0;
			new_block->prev = top;
			top = new_block;
		}
		top->o[top->length++] = obj->retain();
	}
	
	virtual AObject *pop() {
		if (top->length == 0) return NULL;
		AObject *o = top->o[--(top->length)];
		if (top->length == 0 && top->prev) {
			void *last = top;
			top = top->prev;
			free(last);
		}
		return o;
	}
	
	virtual void popAll() {
		while (top->length) {
			vsize_t n = top->length;
			for (vsize_t i = 0; i < n; i++)
				top->o[i]->release();
			if (top->prev) {
				void *last = top;
				top = top->prev;
				free(last);
			} else top->length = 0;				
		}
	}
	
	virtual AObject *peek() {
		return (top->length == 0) ? NULL : top->o[top->length - 1];
	}
	
	virtual bool isEmpty() {
		return (top->length == 0);
	}
	
	virtual bool isLast() {
		return (top->length == 1 && top->prev == NULL);
	}
};

#endif
