/*
 *  AStack.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/5/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

/** Important note! All stack implementations transfer ownership on pop() so it becomes the responsibility of the caller to release the returned object! */

#ifndef A_STACK_H_
#define A_STACK_H_


/** General interface for all stack implementations */
class AStack : public AObject {
public:
	virtual void push(AObject* obj) = 0;
	virtual AObject *pop()  = 0;
	virtual void popAll()   = 0;
	virtual AObject *peek() = 0;
	virtual bool isEmpty()  = 0;
	virtual bool isLast()   = 0;
};

typedef struct AStackBlock {
	vsize_t capacity, length;
	struct AStackBlock *prev;
	AObject *o[1];
} AStackBlock_t;


class ABlockStack : public AStack {
protected:
	AStackBlock_t *top;
	vsize_t default_capacity;
	
public:
	ABlockStack(vsize_t increments = 128) {
		default_capacity = increments;
		if (default_capacity < 16) default_capacity = 16;
		top = (AStackBlock_t*) AAlloc(sizeof(AStackBlock_t) + sizeof(AObject*) * default_capacity);
		AMEM(top);
		top->capacity = default_capacity;
		top->length = 0;
		top->prev = 0;
		OCLASS(ABlockStack)
	}
	
	virtual ~ABlockStack() {
		popAll();
		AFree(top);
		DCLASS(ABlockStack)
	}

	virtual void push(AObject* obj) {
		if (!obj) return;
		if (top->length >= top->capacity) {
			AStackBlock_t *new_block = (AStackBlock_t*) AAlloc(sizeof(AStackBlock_t) + sizeof(AObject*) * default_capacity);
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
			AFree(last);
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
				AFree(last);
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
	
	virtual char *describe() {
		vsize_t n = 0, b = 0;
		AStackBlock_t *cb = top;
		while (cb) {
			n += cb -> length;
			cb = cb -> prev;
			b++;
		}
#ifdef ODEBUG
		snprintf(desc_buf, 512, "<%p/%d %04x %s [%d] %d elements, %d blocks>", this, refcount, _objectSerial, _className, _classSize, n, b);
#else
		snprintf(desc_buf, 512, "<ABlockStack %p %d elements (%d blocks)>", this, n, b);
#endif
		return desc_buf;
	}
	
};

class AForgetfulStack : public AStack {
protected:
	AObject **stack;
	vsize_t top_, size_;
public:
	AForgetfulStack(vsize_t size) : size_(size) {
		top_ = 0;
		if (size_ < 1) size_ = 1;
		stack = (AObject**) AAlloc(sizeof(AObject*) * size_);
		AMEM(stack);
		OCLASS(AForgetfulStack)
	}
	
	virtual ~AForgetfulStack() {
		popAll();
		AFree(stack);
		DCLASS(AForgetfulStack)
	}
	
	virtual void popAll() {
		while (top_) {
			if (stack[top_])
				stack[top_]->release();
			top_--;
		}
	}
	
	virtual bool isEmpty() {
		return (top_ == 0);
	}
	
	virtual bool isLast() {
		return (top_ == 1);
	}
	
	virtual AObject* peek() {
		return top_ ? stack[top_ - 1] : 0;
	}
	
	virtual void push(AObject *obj) {
		if (top_ == size_) {
			if (stack[0])
				stack[0]->release();
			memmove(stack, stack + 1, sizeof(AObject*) * size_);
			top_--;
		}
		if (obj)
			obj->retain();
		stack[top_++] = obj;
	}
	
	virtual AObject* pop() {
		if (top_)
			return stack[--top_];
		return 0;
	}
	
	virtual char *describe() {
#ifdef ODEBUG
		snprintf(desc_buf, 512, "<%p/%d %04x %s [%d] %d/%d elements>", this, refcount, _objectSerial, _className, _classSize, top_, size_);
#else
		snprintf(desc_buf, 512, "<AForgetfulStack %p %d/%d>", this, top_, size_);
#endif
		return desc_buf;
	}
	
};

#endif
