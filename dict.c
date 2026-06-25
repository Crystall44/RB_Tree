#include "dict.h"
#include <stdlib.h>

// Все функции словаря - просто обертки над операциями красно-черного дерева, которое уже хранит пары
Dict* dict_create(void) {
	Dict* d = malloc(sizeof(*d));
	d->tree = rb_create(); // Создаем пустое дерево
	return d;
}

void dict_destroy(Dict* dict) {
	rb_destroy(dict->tree); // Удаляем дерево
	free(dict); // И структуру словаря
}

int dict_put(Dict* dict, int key, int value) {
	return rb_insert(dict->tree, key, value);
}

int dict_get(const Dict* dict, int key, int* out) {
	return rb_find(dict->tree, key, out);
}

int dict_contains(const Dict* dict, int key) {
	return rb_contains(dict->tree, key);
}

int dict_remove(Dict* dict, int key) {
	return rb_remove(dict->tree, key);
}

int dict_size(const Dict* dict) {
	return rb_size(dict->tree);
}

void dict_print(const Dict* dict) {
	rb_print_in_order(dict->tree);
}