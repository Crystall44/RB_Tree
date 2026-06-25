#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "rb_tree.h"
#include "dict.h"

// Счетчики
static int passed = 0;
static int failed = 0;

// Проверка тестов
static void check(const char* description, int condition) {
    if (condition) {
        printf("Успешно %s\n", description);
        passed++;
    }
    else {
        printf("Ошибка %s\n", description);
        failed++;
    }
}

// Тест 1 - создание и удаление красно-черного дерева
static void test_rb_create_destroy(void) {
    printf("\nТест 1: создание и удаление КЧ-дерева\n");

    // 1.1 Создание
    printf("1.1 Создание дерева\n");
    RBTree* t = rb_create();
    check("дерево создано (не NULL)", t != NULL);
    check("размер == 0", rb_size(t) == 0);
    check("nil-страж чёрный", t->nil->color == BLACK);
    check("корень == nil (пустое дерево)", t->root == t->nil);

    // 1.2 Свойства пустого дерева
    printf("\n1.2 Свойства пустого дерева\n");
    check("КЧ-свойства выполняются", rb_check_properties(t));
    check("чёрная высота == 0", rb_black_height(t) == 0);

    // 1.3 Операции на пустом
    printf("\n1.3 Операции на пустом дереве\n");
    check("contains(5) == 0", !rb_contains(t, 5));
    int val = 999;
    check("find(5) == 0", !rb_find(t, 5, &val));
    check("значение не изменилось", val == 999);
    check("remove(5) == 0", !rb_remove(t, 5));

    rb_destroy(t);
    check("дерево удалено без ошибок", 1);
}

// Тест 2 - вставка и свойства КЧ-дерева
static void test_rb_insert(void) {
    printf("\nТест 2: вставка в КЧ-дерево\n");

    RBTree* t = rb_create();

    // 2.1 Вставка одного элемента
    printf("2.1 Вставка одного элемента\n");
    check("insert(10, 100) == 1 (вставлен)", rb_insert(t, 10, 100) == 1);
    check("размер == 1", rb_size(t) == 1);
    check("корень чёрный", t->root->color == BLACK);
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // 2.2 Вставка нескольких
    printf("\n2.2 Вставка нескольких элементов\n");
    rb_insert(t, 5, 50);
    rb_insert(t, 15, 150);
    rb_insert(t, 3, 30);
    rb_insert(t, 7, 70);
    check("размер == 5", rb_size(t) == 5);
    check("КЧ-свойства выполняются", rb_check_properties(t));
    check("чёрная высота > 0", rb_black_height(t) > 0);

    // 2.3 Поиск
    printf("\n2.3 Поиск после вставки\n");
    int val;
    check("find(10) == 100", rb_find(t, 10, &val) && val == 100);
    check("find(5) == 50", rb_find(t, 5, &val) && val == 50);
    check("find(7) == 70", rb_find(t, 7, &val) && val == 70);
    check("find(99) == 0", !rb_find(t, 99, &val));

    // 2.4 Вставка дубликата (обновление) 
    printf("\n2.4 Вставка дубликата (обновление значения)\n");
    check("insert(10, 999) == 0 (обновлён)", rb_insert(t, 10, 999) == 0);
    check("размер не изменился (5)", rb_size(t) == 5);
    check("find(10) == 999", rb_find(t, 10, &val) && val == 999);
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // 2.5 Вставка по возрастанию (стресс-тест балансировки)
    printf("\n2.5 Вставка по возрастанию (1..20)\n");
    RBTree* t2 = rb_create();

    for (int i = 1; i <= 20; i++) {
        rb_insert(t2, i, i * 10);
    }

    check("размер == 20", rb_size(t2) == 20);
    check("КЧ-свойства выполняются", rb_check_properties(t2));
    check("чёрная высота <= 6 (сбалансировано)", rb_black_height(t2) <= 6);

    // Проверяем все значения 
    int all_ok = 1;
    for (int i = 1; i <= 20; i++) {
        if (!rb_find(t2, i, &val) || val != i * 10) {
            all_ok = 0; break;
        }
    }
    check("все 20 значений найдены", all_ok);

    // 2.6 Вставка по убыванию 
    printf("\n2.6 Вставка по убыванию (20..1)\n");
    RBTree* t3 = rb_create();
    for (int i = 20; i >= 1; i--) {
        rb_insert(t3, i, i * 100);
    }
    check("размер == 20", rb_size(t3) == 20);
    check("КЧ-свойства выполняются", rb_check_properties(t3));

    all_ok = 1;
    for (int i = 1; i <= 20; i++) {
        if (!rb_find(t3, i, &val) || val != i * 100) {
            all_ok = 0; break;
        }
    }
    check("все 20 значений найдены", all_ok);

    rb_destroy(t);
    rb_destroy(t2);
    rb_destroy(t3);
}

// Тест 3 - удаление из красно-черного дерева
static void test_rb_remove(void) {
    printf("\nТест 3: удаление из КЧ-дерева\n");

    RBTree* t = rb_create();

    // Строим дерево: 50, 30, 70, 20, 40, 60, 80 
    int keys[] = { 50, 30, 70, 20, 40, 60, 80 };

    for (int i = 0; i < 7; i++) {
        rb_insert(t, keys[i], keys[i] * 10);
    }

    check("начальный размер == 7", rb_size(t) == 7);
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // 3.1 Удаление листа (20) 
    printf("\n3.1 Удаление листа (20)\n");
    check("remove(20) == 1", rb_remove(t, 20));
    check("размер == 6", rb_size(t) == 6);
    check("contains(20) == 0", !rb_contains(t, 20));
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // 3.2 Удаление узла с одним потомком 
    printf("\n3.2 Удаление узла с одним потомком (30)\n");

    // После удаления 20 у 30 только правый потомок 40 
    check("remove(30) == 1", rb_remove(t, 30));
    check("размер == 5", rb_size(t) == 5);
    check("contains(30) == 0", !rb_contains(t, 30));
    check("contains(40) == 1", rb_contains(t, 40));
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // 3.3 Удаление узла с двумя потомками 
    printf("\n3.3 Удаление корня (50) - два потомка\n");
    check("remove(50) == 1", rb_remove(t, 50));
    check("размер == 4", rb_size(t) == 4);
    check("contains(50) == 0", !rb_contains(t, 50));
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // Новый корень должен быть чёрным 
    check("новый корень чёрный", t->root->color == BLACK);

    // 3.4 Повторное удаление 
    printf("\n3.4 Повторное удаление того же ключа\n");
    check("remove(20) == 0 (уже удалён)", !rb_remove(t, 20));
    check("remove(30) == 0 (уже удалён)", !rb_remove(t, 30));

    // 3.5 Удаление несуществующего 
    printf("\n3.5 Удаление несуществующего ключа\n");
    check("remove(999) == 0", !rb_remove(t, 999));
    check("размер не изменился (4)", rb_size(t) == 4);

    // 3.6 Удаление всех до пустого 
    printf("\n3.6 Удаление всех элементов\n");
    rb_remove(t, 40);
    rb_remove(t, 60);
    rb_remove(t, 70);
    rb_remove(t, 80);
    check("размер == 0", rb_size(t) == 0);
    check("корень == nil", t->root == t->nil);
    check("КЧ-свойства выполняются", rb_check_properties(t));

    rb_destroy(t);
}

// Стресс-тест красно-черного дерева (много случайных операций)
static void test_rb_stress(void) {
    printf("\nТест 4: стресс-тест КЧ-дерева\n");

    RBTree* t = rb_create();
    srand(42);

    // 4.1 1000 случайных вставок 
    printf("4.1 1000 случайных вставок\n");
    int inserted = 0;
    int ref[1000] = { 0 }; 

    for (int k = 0; k < 2000 && inserted < 1000; k++) {
        int key = rand() % 3000;
        int value = rand() % 10000;
        int res = rb_insert(t, key, value);
        if (res == 1) {
            ref[inserted] = key;
            inserted++;
        }
    }

    check("вставлено около 1000 элементов", rb_size(t) > 900);
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // 4.2 Проверка всех вставленных 
    printf("\n4.2 Проверка всех вставленных ключей\n");
    int ok = 1;

    for (int i = 0; i < inserted; i++) {
        if (!rb_contains(t, ref[i])) { ok = 0; break; }
    }

    check("все вставленные ключи найдены", ok);

    // 4.3 500 удалений 
    printf("\n4.3 Удаление 500 элементов\n");
    for (int i = 0; i < 500; i++) {
        rb_remove(t, ref[i]);
    }

    check("размер уменьшился", rb_size(t) <= inserted - 450);
    check("КЧ-свойства выполняются", rb_check_properties(t));

    // 4.4 Вставка
    printf("\n4.4 Вставка после удалений\n");

    for (int i = 0; i < 200; i++) {
        rb_insert(t, rand() % 3000, rand() % 10000);
    }

    check("КЧ-свойства выполняются", rb_check_properties(t));

    rb_destroy(t);
    check("стресс-тест пройден", 1);
}

// Тест 5 - Создание и базовые операции словаря
static void test_dict_basic(void) {
    printf("\nТест 5: словарь - базовые операции\n");

    // 5.1 Создание 
    printf("5.1 Создание словаря\n");
    Dict* d = dict_create();

    check("словарь создан", d != NULL);
    check("размер == 0", dict_size(d) == 0);

    // 5.2 Вставка 
    printf("\n5.2 Вставка элементов\n");

    check("put(1,10) -> вставлен", dict_put(d, 1, 10) == 1);
    check("put(3,30) -> вставлен", dict_put(d, 3, 30) == 1);
    check("put(2,20) -> вставлен", dict_put(d, 2, 20) == 1);
    check("put(5,50) -> вставлен", dict_put(d, 5, 50) == 1);
    check("put(4,40) -> вставлен", dict_put(d, 4, 40) == 1);

    check("размер == 5", dict_size(d) == 5);

    // 5.3 Поиск 
    printf("\n5.3 Поиск\n");
    int val;

    check("get(1) == 10", dict_get(d, 1, &val) && val == 10);
    check("get(2) == 20", dict_get(d, 2, &val) && val == 20);
    check("get(3) == 30", dict_get(d, 3, &val) && val == 30);
    check("get(4) == 40", dict_get(d, 4, &val) && val == 40);
    check("get(5) == 50", dict_get(d, 5, &val) && val == 50);

    check("get(99) == 0 (нет)", !dict_get(d, 99, &val));

    // 5.4 Обновление 
    printf("\n5.4 Обновление существующего ключа\n");

    check("put(3,333) -> обновлён", dict_put(d, 3, 333) == 0);
    check("get(3) == 333", dict_get(d, 3, &val) && val == 333);
    check("размер не изменился (5)", dict_size(d) == 5);

    // 5.5 contains 
    printf("\n5.5 Проверка наличия\n");

    check("contains(1) == 1", dict_contains(d, 1));
    check("contains(5) == 1", dict_contains(d, 5));
    check("contains(99) == 0", !dict_contains(d, 99));

    // 5.6 Удаление 
    printf("\n5.6 Удаление\n");
    check("remove(2) == 1", dict_remove(d, 2));
    check("размер == 4", dict_size(d) == 4);
    check("contains(2) == 0", !dict_contains(d, 2));
    check("remove(2) == 0 (повторно)", !dict_remove(d, 2));

    check("remove(1) == 1", dict_remove(d, 1));
    check("remove(5) == 1", dict_remove(d, 5));
    check("размер == 2", dict_size(d) == 2);

    // 5.7 Вставка после удаления 
    printf("\n5.7 Вставка после удаления\n");
    check("put(2,200) -> вставлен", dict_put(d, 2, 200) == 1);
    check("get(2) == 200", dict_get(d, 2, &val) && val == 200);
    check("размер == 3", dict_size(d) == 3);

    dict_print(d);
    dict_destroy(d);
}

// Тест 6 - словарь, много элементов
static void test_dict_large(void) {
    printf("\nТест 6: словарь - большой объём\n");

    Dict* d = dict_create();
    srand(123);

    // 6.1 Вставка 500 элементов
    printf("6.1 Вставка 500 элементов\n");
    int keys[500];

    for (int i = 0; i < 500; i++) {
        keys[i] = rand() % 10000;
        dict_put(d, keys[i], keys[i] * 10);
    }

    check("размер > 0", dict_size(d) > 0);

    // 6.2 Поиск случайных 
    printf("\n6.2 Поиск 100 случайных ключей\n");
    int found = 0;

    for (int i = 0; i < 100; i++) {
        int idx = rand() % 500;
        if (dict_contains(d, keys[idx])) found++;
    }

    check("найдено около 100", found >= 90);

    // 6.3 Удаление 200 
    printf("\n6.3 Удаление 200 элементов\n");
    size_t before = dict_size(d);

    for (int i = 0; i < 200; i++) {
        dict_remove(d, keys[i]);
    }

    check("размер уменьшился", dict_size(d) < before);

    dict_destroy(d);
}

// Примеры из жизни
static void test_real_world(void) {
    printf("\nТест 7: примеры из жизни\n");

    // 7.1 Словарь как телефонная книга 
    printf("7.1 Телефонная книга (номер сотрудника -> имя)\n");
    Dict* phonebook = dict_create();

    dict_put(phonebook, 101, 0);  // 0 будет означать Алиса
    dict_put(phonebook, 102, 1);  // 1 - Борис
    dict_put(phonebook, 103, 2);  // 2 - Виктор 
    dict_put(phonebook, 201, 3);  // 3 - Галина 
    dict_put(phonebook, 202, 4);  // 4 - Дмитрий 

    const char* names[] = { "Алиса", "Борис", "Виктор", "Галина", "Дмитрий" };

    check("размер == 5", dict_size(phonebook) == 5);

    int name_id;
    dict_get(phonebook, 103, &name_id);
    check("номер 103 - Виктор", name_id == 2);

    check("номер 999 - нет", !dict_contains(phonebook, 999));

    // Смена номера 
    dict_remove(phonebook, 103);
    dict_put(phonebook, 303, 2);    
    check("Виктор переехал на 303", dict_contains(phonebook, 303));
    check("старый номер 103 свободен", !dict_contains(phonebook, 103));

    printf("Телефонная книга: ");
    dict_print(phonebook);
    dict_destroy(phonebook);

    // 7.2 Словарь как кэш (ключ -> результат вычисления) 
    printf("\n7.2 Кэш вычислений (число -> квадрат)\n");
    Dict* cache = dict_create();

    // Имитация: вычисляем квадраты и сохраняем в кэш 
    for (int i = 1; i <= 10; i++) {
        dict_put(cache, i, i * i);
    }

    int square;
    dict_get(cache, 7, &square);
    check("квадрат 7 == 49", square == 49);

    dict_get(cache, 10, &square);
    check("квадрат 10 == 100", square == 100);

    check("квадрат 11 не в кэше", !dict_contains(cache, 11));

    printf("Содержимое кэша: ");
    dict_print(cache);
    dict_destroy(cache);

    // 7.3 Упорядоченный словарь: рейтинг студентов 
    printf("\n7.3 Рейтинг студентов (студент ID - балл)\n");
    Dict* rating = dict_create();

    // ID студента -> балл за экзамен 
    dict_put(rating, 1001, 85);
    dict_put(rating, 1002, 92);
    dict_put(rating, 1003, 78);
    dict_put(rating, 1004, 95);
    dict_put(rating, 1005, 88);

    check("размер == 5", dict_size(rating) == 5);

    // Благодаря КЧ-дереву ключи хранятся упорядоченно 
    printf("Рейтинг (по возрастанию ID): ");
    dict_print(rating);

    // Обновление балла 
    dict_put(rating, 1003, 82);  
    int score;

    dict_get(rating, 1003, &score);
    check("студент 1003 повысил балл до 82", score == 82);

    // Отчисление 
    dict_remove(rating, 1001);
    check("студент 1001 отчислен", !dict_contains(rating, 1001));
    check("осталось 4 студента", dict_size(rating) == 4);

    dict_destroy(rating);

    // 7.4 Инвертированный индекс (слово - частота) 
    printf("\n7.4 Частотный анализ слов\n");
    Dict* freq = dict_create();

    // Сопоставим словам числовые коды 
    // 1=hello, 2=world, 3=apple, 4=hello (повтор)...
    int words[] = { 1, 2, 1, 3, 1, 2, 4, 1, 3, 2 };
    int n = 10;

    for (int i = 0; i < n; i++) {
        int cnt = 0;
        dict_get(freq, words[i], &cnt);
        dict_put(freq, words[i], cnt + 1);
    }

    int cnt;
    dict_get(freq, 1, &cnt);
    check("слово 1 (hello) встретилось 4 раза", cnt == 4);

    dict_get(freq, 2, &cnt);
    check("слово 2 (world) встретилось 3 раза", cnt == 3);

    dict_get(freq, 3, &cnt);
    check("слово 3 (apple) встретилось 2 раза", cnt == 2);

    dict_get(freq, 4, &cnt);
    check("слово 4 встретилось 1 раз", cnt == 1);


    printf("Частоты: ");
    dict_print(freq);
    dict_destroy(freq);
}


int main(void) {
    setlocale(LC_ALL, "Russian");
    printf("Тесты: Красно-чёрное дерево и Словарь\n");

    // Тесты дерева 
    test_rb_create_destroy();
    test_rb_insert();
    test_rb_remove();
    test_rb_stress();

    // Тесты словаря 
    test_dict_basic();
    test_dict_large();

    // Примеры из жизни 
    test_real_world();

    printf("Итоги:\n");
    printf("Пройдено: %d\n", passed);
    printf("Провалено: %d\n", failed);
    printf("Всего: %d\n", passed + failed);

    if (failed == 0) {
        printf("\nВсе тесты пройдены успешно!\n");
    }
    else {
        printf("\nЕсть проваленные тесты...\n");
    }

    return 0;
}