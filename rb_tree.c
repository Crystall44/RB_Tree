#include "rb_tree.h"
#include <stdlib.h>
#include <stdio.h>

// Вспомогательные функции

// Создание нового узла. По умолчанию узел всегда красный, левый и правый потомки указывают на nil-страж
static RBNode* node_create(RBTree* t, int key, int value) {
	RBNode* n = malloc(sizeof(*n));
	n->key = key;
	n->value = value;
	n->color = RED;
	n->left = t->nil;
	n->right = t->nil;
	n->parent = t->nil;
	return n;
}

// Левый поворот вокруг узла x
// Используется для восстановления красно-черных инвариантов
// Правое поддерево x (узел y) поднимается наверх, x становится левым потомком y, а бывшее левое поддерево y
// становится правым поддеревом x.
static void left_rotate(RBTree* t, RBNode* x) {
	RBNode* y = x->right; // y - правое поддерево x
	x->right = y->left; // левое поддерево y становится правым поддеревом x

	if (y->left != t->nil) y->left->parent = x; // обновляем родителя у перемещенного поддерева

	y->parent = x->parent; // родитель x становится родителем y
	if (x->parent == t->nil) t->root = y; // x был корнем, теперь корнем становится y
	else if (x == x->parent->left) {
		x->parent->left = y; // x был левым потомком, y теперь левый потомок
	}
	else {
		x->parent->right = y; // x был правым потомком, y теперь правый потомок
	}

	y->left = x; // x становится левым потомком y
	x->parent = y; // y становится родителем x
}

// Правый поворот вокруг узла x
// Симметричен левому повороту: левое поддерево x (узел y) поднимается наверх,
// x становится правым потомком y, а бывшее правое поддерево y становится левым поддеревом x.
static void right_rotate(RBTree* t, RBNode* x) {
	RBNode* y = x->left; // y — левое поддерево x 
	x->left = y->right; // правое поддерево y становится левым поддеревом x 

	if (y->right != t->nil)
		y->right->parent = x; // обновляем родителя у перемещённого поддерева 

	y->parent = x->parent; // родитель x теперь становится родителем y 

	if (x->parent == t->nil)
		t->root = y; // x был корнем — теперь корнем становится y 
	else if (x == x->parent->right)
	{
		x->parent->right = y;// x был правым потомком, y теперь правый потомок 
	}
	else {
		x->parent->left = y; // x был левым потомком, y теперь левый потомок 
	}
	
	y->right = x; // x становится правым потомком y 
	x->parent = y; // y становится родителем x 
}

// Восстановление красно-черных свойств после вставки
// Нарушение: у красного узла z красный родитель
// Три случая разбираются относительно "дяди" (брата родителя)
static void insert_fixup(RBTree* t, RBNode* z) {
	// Пока родитель красный - нарушение
	while (z->parent->color == RED) {
		if (z->parent == z->parent->parent->left) {
			// Родитель z левый потомок деда
			RBNode* y = z->parent->parent->right; //дядя

			if (y->color == RED) {
				// Случай 1 - дядя красный
				// Перекрашиваем отца, дядю в черный, деда в красный и поднимаем проблему выше
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			}
			else {
				// Дядя черный
				if (z == z->parent->right) {
					// Случай 2 - z правый потомок (зигзаг)
					// Поворачиваем влево вокруг родителя, сводя к случаю 3
					z = z->parent;
					left_rotate(t, z);
				}
				// Случай 3 - z левый потомок (линия)
				// Перекрашиваем и делаем правый поворот вокруг "деда"
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				right_rotate(t, z->parent->parent);
			}
		}
		else {
			// Симметричный случай: родитель z правый потомок деда
			RBNode* y = z->parent->parent->left; // дядя

			if (y->color == RED) {
				// Случай 1
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			}
			else {
				// Дядя черный
				if (z == z->parent->left) {
					// Случай 2 
					z = z->parent;
					right_rotate(t, z);
				}
				// Случай 3
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				left_rotate(t, z->parent->parent);
			}
		}
	}

	// Корень всегда должен быть черным
	t->root->color = BLACK;
}

// Публичные функции

// Создание пустого красно-черного дерева. nil страж - единственный черный узел, на который ссылаются все листья
RBTree* rb_create(void) {
	RBTree* t = malloc(sizeof(*t));
	t->nil = malloc(sizeof(RBNode));
	t->nil->color = BLACK;
	t->nil->left = t->nil->right = t->nil->parent = NULL;
	t->root = t->nil; // Пустое дерево - корень = страж
	t->size = 0;
	return t;
}

// Рекурсивное удаление поддерева
static void destroy_subtree(RBTree* t, RBNode* n) {
	if (n == t->nil) return;
	destroy_subtree(t, n->left);
	destroy_subtree(t, n->right);
	free(n);
}

// Уничтожение всего дерева
void rb_destroy(RBTree* t) {
	destroy_subtree(t, t->root);
	free(t->nil);
	free(t);
}

// Вставка пары key, value
// Сначала стандартная вставка, затем восстановление свойств
int rb_insert(RBTree* t, int key, int value) {
	RBNode* z = node_create(t, key, value); // Новый красный узел
	RBNode* y = t->nil; // Будущий родитель z
	RBNode* x = t->root; // Текущий узел при спуске

	// Спуск по дереву
	while (x != t->nil) {
		y = x;
		if (key == x->key) {
			// Ключ уже есть - обновляем значение
			x->value = value;
			free(z);
			return 0; // обновлен
		} 
		if (key < x->key) {
			x = x->left;
		}
		else {
			x = x->right;
		}
	}

	// Подвешиваем z к родителю y
	z->parent = y;
	if (y == t->nil) {
		t->root = z; // Дерево было пустым
	}
	else if (key < y->key) {
		y->left = z;
	}
	else {
		y->right = z;
	}

	t->size++;

	// Восстанавливаем красно-черные свойства
	insert_fixup(t, z);
	return 1; // успешная вставка
}

// Поиск узла по ключу 
// Возвращает указатель на узел или t->nil если не найден
static RBNode* search_node(const RBTree* t, int key) {
	RBNode* current = t->root;
	while (current != t->nil && current->key != key) {
		if (key < current->key) {
			current = current->left;
		}
		else current = current->right;
	}

	return current;
}

// Поиск значения по ключу
// Возвращает 1 и записывает значение в *out, если ключ найден
int rb_find(const RBTree* t, int key, int* out) {
	RBNode* n = search_node(t, key);
	if (n == t->nil) return 0;
	*out = n->value;
	return 1;
}

// Проверка наличия ключа в дереве
int rb_contains(const RBTree* t, int key) {
	return search_node(t, key) != t->nil;
}

// Удаление узла - замена поддерева u поддеревом v
// Поддерево v встает на место u
static void replace(RBTree* t, RBNode* u, RBNode* v) {
	if (u->parent == t->nil) {
		t->root = v; // u был корнем - теперь корнем становится v
	}
	else if (u == u->parent->left) {
		u->parent->left = v; // u был левым потомком - v становится на его место
	}
	else {
		u->parent->right = v; // u был правым потомком - v становится на его место
	}
	v->parent = u->parent; // родитель u теперь родитель v (даже если это nil)
}

// Поиск узла с минимальным ключом в поддереве
static RBNode* minimum(RBTree* t, RBNode* n) {
	while (n->left != t->nil) {
		n = n->left;
	}
	return n;
}

// Восстановление красно-черных свойств после удаления
// Вызывается, если удаленный узел был черным, что нарушило черную высоту
static void delete_fixup(RBTree* t, RBNode* x) {
	// Пока x не корень и он не черный
	while (x != t->root && x->color == BLACK) {
		if (x == x->parent->left) {
			RBNode* w = x->parent->right; // брат x

			// Случай 1 - брат красный
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				left_rotate(t, x->parent);
				w = x->parent->right; // новый брат стал черным
			}

			// Случай 2 - оба племянника черные
			if (w->left->color == BLACK && w->right->color == BLACK) {
				w->color = RED; // снимаем лишнюю черноту
				x = x->parent; // Передаем проблему выше
			}
			else {
				// Случай 3 - правый племянник черный (левый красный)
				if (w->right->color == BLACK) {
					w->left->color = BLACK;
					w->color = RED;
					right_rotate(t, w);
					w = x->parent->right; // обновляем
				}

				// Случай 4 - правый племянник красный
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				left_rotate(t, x->parent);
				x = t->root; // проблема решена, выходим
			}
		}
		else {
			// Симметрично для правой стороны
			RBNode* w = x->parent->left; // брат x

			// Случай 1 - брат красный
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				right_rotate(t, x->parent);
				w = x->parent->left; 
			}

			// Случай 2 - оба племянника черные
			if (w->left->color == BLACK && w->right->color == BLACK) {
				w->color = RED; // снимаем лишнюю черноту
				x = x->parent; // Передаем проблему выше
			}
			else {
				// Случай 3 - левый племянник черный (правый красный)
				if (w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					left_rotate(t, w);
					w = x->parent->left; // обновляем
				}

				// Случай 4 - левый племянник красный
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				right_rotate(t, x->parent);
				x = t->root; // проблема решена, выходим
			}
		}
	}

	x->color = BLACK; // корень должен быть черным
}

// Удаление узла по ключу
int rb_remove(RBTree* t, int key) {
	RBNode* z = search_node(t, key);
	if (z == t->nil) return 0; // ключ не найден

	RBNode* y = z; // y - узел, который будет удален или перемещён
	RBNode* x; // x - узел, который займет место y
	Color y_orig_color = y->color;

	// Три случая удаления
	if (z->left == t->nil) {
		// Левого потомка нет - переставляем правое поддерево
		x = z->right;
		replace(t, z, z->right);
	}
	else if (z->right == t->nil) {
		// Правого потомка нет - переставляем левое поддерево
		x = z->left;
		replace(t, z, z->left);
	}
	else {
		// Оба потомка есть - находим преемника (мин. в правом дереве)
		y = minimum(t, z->right);
		y_orig_color = y->color;
		x = y->right;

		if (y->parent == z) {
			// Преемник - потомок z
			x->parent = y;
		}
		else {
			// Преемник глубже
			replace(t, y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}

		// Ставим преемника на место z
		replace(t, z, y);
		y->left = z->left;
		y->left->parent = y;
		y->color = z->color;
	}

	free(z);
	t->size--;

	// Если удаленный или перемещенный узел был черным - нужна балансировка
	if (y_orig_color == BLACK) {
		delete_fixup(t, x);
	}

	return 1;
}

// Текущий размер дерева
int rb_size(const RBTree* t) {
	return t->size;
}

// Рекурсивный обход по порядку для печати
static void in_order(RBTree* t, RBNode* n) {
	if (n == t->nil) return;
	in_order(t, n->left);
	printf("(%d:%d) ", n->key, n->value);
	in_order(t, n->right);
}

// Печать всех пар в порядке возрастания ключей
void rb_print_in_order(const RBTree* t) {
	in_order(t, t->root);
	printf("\n");
}

// Рекурсивный обход 
static int check_node(const RBTree* t, RBNode* n, int* bh) {
	if (n == t->nil) {
		*bh = 1;   // nil-лист считается чёрным, высота = 1
		return 1;
	}

	// Свойство BST: left < key < right
	if (n->left != t->nil && n->left->key >= n->key) return 0;
	if (n->right != t->nil && n->right->key <= n->key) return 0;

	// Свойство 4: у красного узла оба потомка чёрные
	if (n->color == RED) {
		if (n->left->color != BLACK || n->right->color != BLACK) return 0;
	}

	int lbh, rbh;
	if (!check_node(t, n->left, &lbh)) return 0;
	if (!check_node(t, n->right, &rbh)) return 0;

	// Свойство 5: одинаковая чёрная высота
	if (lbh != rbh) return 0;

	*bh = lbh + (n->color == BLACK ? 1 : 0);
	return 1;
}

// Проверка всех пяти свойств красно-черного дерева
int rb_check_properties(const RBTree* tree) {
	if (tree == NULL) return 0;

	/* Свойство 2: корень чёрный */
	if (tree->root->color != BLACK) return 0;

	/* Свойство 3: nil-страж чёрный */
	if (tree->nil->color != BLACK) return 0;

	int bh;
	return check_node(tree, tree->root, &bh);
}

// Возвращает черную высоту дерева для отладки
int rb_black_height(const RBTree* tree) {
	if (tree == NULL || tree->root == tree->nil) return 0;
	int bh = 0;
	RBNode* cur = tree->root;
	while (cur != tree->nil) {
		if (cur->color == BLACK) bh++;
		cur = cur->left;
	}
	return bh;
}