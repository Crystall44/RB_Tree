#ifndef DICT_H
#define DICT_H

#include "rb_tree.h"

// Структура словаря - ассоциативного массива
// Обертка над красно-черным деревом. Ключ - int, значение - int
typedef struct {
	RBTree* tree; // Базовое красно-черное дерево
} Dict;

// Создать пустой словарь
Dict* dict_create(void);

// Уничтожить словарь 
void dict_destroy(Dict* dict);

// Вставить или обновить пару key:value
// Возвращает 1 если вставка, 0 если обновление
int dict_put(Dict* dict, int key, int value);

// Получить значение по ключу 
// Возвращает 1 и записывает в *out если найден, 0 если ключа нет
int dict_get(const Dict* dict, int key, int* out);

// Проверить наличие ключа
int dict_contains(const Dict* dict, int key);

// Удалить пару по ключу
int dict_remove(Dict* dict, int key);

// Количество пар в словаре
int dict_size(const Dict* dict);

// Напечатать все пары в порядке возрастания ключей
void dict_print(const Dict* dict);

#endif