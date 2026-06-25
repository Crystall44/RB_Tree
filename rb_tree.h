#ifndef RB_TREE_H
#define RB_TREE_H

// Константы цвета
typedef enum { BLACK, RED } Color;

// Красно-черное дерево - узел
typedef struct RBNode {
	int key; // Ключ, по которому строится дерево
	int value; // Значение, связанное с ключом (для словаря)
	Color color; // Цвет узла
	struct RBNode* left; // Левый потомок
	struct RBNode* right; // Правый потомок
	struct RBNode* parent; // Родитель
} RBNode;

// Структура дерева
typedef struct RBTree {
	RBNode* root; // Корень дерева
	RBNode* nil; // Единый черный страж (sentinel) для всех листьев
	int size; // Количество узлов в дереве
} RBTree;

// Операции

// Создать пустое дерево
RBTree* rb_create(void);

// Уничтожить дерево
void rb_destroy(RBTree* tree);

// Вставка ключа со значением
// Возвращает 1 если ключ вставлен, 0 если уже был(значение обновляется)
int rb_insert(RBTree* tree, int key, int value);

// Поиск значения по ключу
int rb_find(const RBTree* tree, int key, int* out_value);

// Проверка наличия ключа
int rb_contains(const RBTree* tree, int key);

// Удаления ключа. 1 если удален, 0 если не найден
int rb_remove(RBTree* tree, int key);

// Текущий размер дерева
int rb_size(const RBTree* tree);

// Печать всех пар key:value в порядке возрастания ключей
void rb_print_in_order(const RBTree* tree);

#endif


