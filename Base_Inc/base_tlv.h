#ifndef base_tlv_h
#define base_tlv_h

#include "MyTypes.h"

#pragma pack(push, 1)

#define t_type			uint8_t
#define l_type			uint16_t

#define IBYTE(v) { \
	switch (sizeof(v)){ \
	case 2: v = htons(v); break; \
	case 4: v = htonl(v); break; \
	default: break;	} }

#define MAKE_NODE(b,n) {current_node.body = (uint8_t*)b; current_node.length = n; current_node.pos = 0;}
#define SETPOS(v) current_node.pos = v
#define STEP(v) current_node.pos += v
#define IS_END() (current_node.pos >= current_node.length)
#define IS_NODE(v) v.body != NULL

typedef struct tlv_node
{
	uint8_t*	body;
	uint32_t	length;
	uint32_t	pos;
}tlv_node, tlv_child;

typedef struct tlv_head
{
	t_type		T;
	l_type		L;
}tlv_head;

#pragma pack(pop)

static tlv_node	 current_node;


static tlv_head make_head()
{
	tlv_head head = *(tlv_head*)(current_node.body + current_node.pos);
	IBYTE(head.T);
	IBYTE(head.L);
	return head;
}

static uint8_t* current_body()
{
	 return current_node.body + current_node.pos + sizeof(tlv_head);
}

static uint8_t* to_buffer(t_type type, uint32_t* len, uint32_t index = 0)
{
	SETPOS(0);
	do 
	{
		tlv_head head = make_head();
		if (head.T == type && index-- == 0)
		{
			*len = head.L;
			return current_body();
		}
		else
		{
			STEP(sizeof(tlv_head));
			STEP(head.L);
		}

	} while (!IS_END());

	return NULL;
}

static tlv_child to_object(t_type type, uint32_t index = 0)
{
	tlv_child child;
	uint32_t len = 0;
	child.body = to_buffer(type, &len, index);
	return child;
}

static const char* to_string(t_type type, uint32_t index = 0)
{
	uint32_t len = 0;
	return (const char*)to_buffer(type, &len, index);
}

static int32_t to_integer(t_type type, uint32_t index = 0)
{
	uint32_t len = 0;
	uint8_t* buffer = to_buffer(type, &len, index);
	if (buffer == NULL)
		return 0;

	switch (len)
	{
	case 1:
		return (*(uint8_t*)buffer);
		break;
	case 2:
		return ntohs(*(uint16_t*)buffer);
		break;
	case 4:
		return ntohl(*(uint32_t*)buffer);
		break;
	default:
		return 0;
		break;
	}
}

static int8_t to_int8(t_type type, uint32_t index = 0)
{
	return (int8_t)to_integer(type, index);
}

static uint8_t to_uint8(t_type type, uint32_t index = 0)
{
	return (uint8_t)to_integer(type, index);
}

static int16_t to_int16(t_type type, uint32_t index = 0)
{
	return (int16_t)to_integer(type, index);
}

static uint16_t to_uint16(t_type type, uint32_t index = 0)
{
	return (uint16_t)to_integer(type, index);
}

static int32_t to_int32(t_type type, uint32_t index = 0)
{
	return (int32_t)to_integer(type, index);
}

static uint32_t to_uint32(t_type type, uint32_t index = 0)
{
	return (uint32_t)to_integer(type, index);
}



#endif //base_tlv_h