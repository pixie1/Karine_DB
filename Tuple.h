#pragma once
#ifndef TUPLE_H
#define TUPLE_H

const int MAXTUPLE = 96;
struct Tuple
{
	long int key; 
	char value[MAXTUPLE];
	//long int value; 
	Tuple();
	Tuple(long int);
	Tuple(long int, char*);
	void set(long int, char*);
	void set(long int k){ key = k; value[0] = '\0'; };
	bool operator==(Tuple&);
};
#endif

inline bool Tuple::operator==(Tuple& source){
	if (this->key == source.key && this->value == source.value)
		return true;
	else
		return false;
}

inline Tuple::Tuple(long int k, char* _value)
{
	key = k;
	for (int i = 0; i < MAXTUPLE; ++i)
		value[i] = _value[i];
}

inline Tuple::Tuple(long int k){
	key = k; 
	value[0] = '\0';
}
inline void Tuple::set(long int k,char* _value)
{
	key = k;
	for (int i = 0; i < MAXTUPLE; ++i)
		value[i] = _value[i];
}

inline Tuple::Tuple()
{
	key = 0;
	value[0]='\0';
}
