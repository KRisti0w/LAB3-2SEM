#include <iostream>
#include <utility> // для std::move

// ==================== ЧАСТЬ 1: MyUnique ====================
template<typename T>
class MyUnique {
    T* ptr = nullptr;

public:
    // Конструктор от сырого указателя
    explicit MyUnique(T* p = nullptr) : ptr(p) {}

    // Деструктор
    ~MyUnique() {
        delete ptr;
    }

    // Запрещаем копирование
    MyUnique(const MyUnique&) = delete;
    MyUnique& operator=(const MyUnique&) = delete;

    // Конструктор перемещения
    MyUnique(MyUnique&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    // Оператор присваивания с перемещением
    MyUnique& operator=(MyUnique&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // Метод get
    T* get() const {
        return ptr;
    }

    // Перегрузка оператора *
    T& operator*() const {
        return *ptr;
    }

    // Перегрузка оператора ->
    T* operator->() const {
        return ptr;
    }
};

// ==================== ЧАСТЬ 2: MyShared ====================
template<typename T>
class MyShared {
    T* ptr = nullptr;
    int* ref_count = nullptr;

public:
    // Конструктор от сырого указателя
    explicit MyShared(T* p = nullptr) : ptr(p), ref_count(new int(1)) {}

    // Конструктор копирования
    MyShared(const MyShared& other) : ptr(other.ptr), ref_count(other.ref_count) {
        if (ref_count) {
            ++(*ref_count);
        }
    }

    // Деструктор
    ~MyShared() {
        release();
    }

    // Оператор присваивания копированием
    MyShared& operator=(const MyShared& other) {
        if (this != &other) {
            release();
            ptr = other.ptr;
            ref_count = other.ref_count;
            if (ref_count) {
                ++(*ref_count);
            }
        }
        return *this;
    }

    // Конструктор перемещения
    MyShared(MyShared&& other) noexcept : ptr(other.ptr), ref_count(other.ref_count) {
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }

    // Оператор присваивания перемещением
    MyShared& operator=(MyShared&& other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            ref_count = other.ref_count;
            other.ptr = nullptr;
            other.ref_count = nullptr;
        }
        return *this;
    }

    // Метод get
    T* get() const {
        return ptr;
    }

    // Перегрузка оператора *
    T& operator*() const {
        return *ptr;
    }

    // Перегрузка оператора ->
    T* operator->() const {
        return ptr;
    }

    // Метод для получения счётчика ссылок (для демонстрации)
    int use_count() const {
        return ref_count ? *ref_count : 0;
    }

private:
    void release() {
        if (ref_count) {
            --(*ref_count);
            if (*ref_count == 0) {
                delete ptr;
                delete ref_count;
            }
        }
        ptr = nullptr;
        ref_count = nullptr;
    }
};

// ==================== Демонстрация ====================
class TestClass {
public:
    int value;
    TestClass(int v) : value(v) {
        std::cout << "TestClass(" << value << ") создан\n";
    }
    ~TestClass() {
        std::cout << "TestClass(" << value << ") уничтожен\n";
    }
    void print() const {
        std::cout << "Значение: " << value << "\n";
    }
};

int main() {
    std::cout << "========== ДЕМОНСТРАЦИЯ MyUnique ==========\n";
    {
        MyUnique<TestClass> u1(new TestClass(10));
        std::cout << "u1.get() = " << u1.get() << "\n";
        std::cout << "*u1 = "; u1->print();

        // MyUnique<TestClass> u2 = u1; // Ошибка компиляции — копирование запрещено

        MyUnique<TestClass> u2 = std::move(u1); // перемещение
        std::cout << "После перемещения u1.get() = " << u1.get() << "\n";
        std::cout << "u2->value = " << u2->value << "\n";

        MyUnique<TestClass> u3;
        u3 = std::move(u2);
        std::cout << "Ещё одно перемещение, u3->value = " << u3->value << "\n";
    } // Здесь уничтожатся u3 (и ранее перемещённые — пустые)

    std::cout << "\n========== ДЕМОНСТРАЦИЯ MyShared ==========\n";
    {
        MyShared<TestClass> s1(new TestClass(20));
        std::cout << "s1 use_count = " << s1.use_count() << "\n";

        MyShared<TestClass> s2 = s1; // копирование
        std::cout << "После копирования s1 use_count = " << s1.use_count() << "\n";
        std::cout << "s2 use_count = " << s2.use_count() << "\n";

        {
            MyShared<TestClass> s3 = s2; // ещё одно копирование
            std::cout << "Внутри блока s3 use_count = " << s3.use_count() << "\n";
        } // s3 уничтожается, счётчик уменьшается

        std::cout << "После выхода из блока s1 use_count = " << s1.use_count() << "\n";

        MyShared<TestClass> s4(new TestClass(30));
        s4 = s1; // присваивание копированием (старый объект s4 удалится)
        std::cout << "После присваивания s4 use_count = " << s4.use_count() << "\n";

        MyShared<TestClass> s5 = std::move(s1); // перемещение
        std::cout << "После перемещения s1 use_count = " << s1.use_count() << "\n";
        std::cout << "s5 use_count = " << s5.use_count() << "\n";

        std::cout << "Работа через операторы * и ->: ";
        (*s5).print();
        s5->print();
    } // Все оставшиеся объекты уничтожатся, память освободится один раз

    return 0;
}