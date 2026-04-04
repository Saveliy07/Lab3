#!/bin/bash

# Сначала собираем проект
make

echo "======================================"
echo " Запуск 20 тестов Калькулятора"
echo "======================================"

passed=0
total=0

run_test() {
    local test_name="$1"
    local expected="$2"
    local input_file="temp_input.txt"
    local output_file="temp_output.txt"

    # Записываем ввод в файл
    cat > "$input_file"
    
    # Запускаем программу
    ./calc < "$input_file" > "$output_file"
    
    # Ищем ожидаемую строку в выводе
    if grep -qF "$expected" "$output_file"; then
        echo "[ УСПЕХ ] $test_name"
        ((passed++))
    else
        echo "[ОШИБКА] $test_name"
        echo "         Ожидалось: '$expected'"
        echo "         Получено :"
        cat "$output_file"
    fi
    ((total++))
}

# --- БЛОК 1: Базовые вычисления ---

run_test "Тест 1: Пример из задания (x*x+y*2)" "17" <<EOF
evaluate
2
x y
3 4
x*x+y*2
EOF

run_test "Тест 2: Тригонометрия и функции" "2" <<EOF
evaluate
1
x
0
sin(x) + cos(x) + exp(x)
EOF

run_test "Тест 3: Приоритет операций и унарный минус" "0" <<EOF
evaluate
0
10 + -5 * 2
EOF

run_test "Тест 4: Возведение в степень" "8" <<EOF
evaluate
0
16^(3/4)
EOF

# --- БЛОК 2: Производные ---

run_test "Тест 5: Вычисление значения производной (x^3 при x=2)" "12" <<EOF
evaluate_derivative
1
x
2
x^3
EOF

run_test "Тест 6: Строка производной sin(x)" "cos" <<EOF
derivative
1
x
0
sin(x)
EOF

# --- БЛОК 3: Поведение пределов (IEEE 754) по чекеру ---

run_test "Тест 7: Деление на ноль (выдает inf)" "inf" <<EOF
evaluate
1
x
0
5 / x
EOF

run_test "Тест 8: Логарифм нуля (выдает -inf)" "-inf" <<EOF
evaluate
0
log(0)
EOF

# --- БЛОК 4: Математические ошибки (Domain Errors) по методичке ---

run_test "Тест 9: Корень из отрицательного числа" "ERROR Domain error: sqrt(-1)" <<EOF
evaluate
1
x
-1
sqrt(x)
EOF

run_test "Тест 10: Логарифм отрицательного числа" "ERROR Domain error: log(x) requires x >= 0" <<EOF
evaluate
0
log(-5)
EOF

run_test "Тест 11: acos вне области определения" "ERROR Domain error: acos(x) requires |x| <= 1" <<EOF
evaluate
0
acos(2)
EOF

run_test "Тест 12: Дробная степень отрицательного числа" "ERROR Domain error: fractional power" <<EOF
evaluate
0
(-2)^0.5
EOF

run_test "Тест 13: Неопределенность 0/0" "ERROR Domain error: 0/0 is undefined" <<EOF
evaluate
0
0 / 0
EOF

# --- БЛОК 5: Лексические и Синтаксические ошибки ---

run_test "Тест 14: Несколько точек в числе" "ERROR: multiple dots in a number" <<EOF
evaluate
0
1.2.3 + 4
EOF

run_test "Тест 15: Ведущие нули запрещены" "ERROR: leading zeros are not allowed" <<EOF
evaluate
0
005 + 1
EOF

run_test "Тест 16: Буква сразу после числа" "ERROR: number cannot be directly followed by a letter" <<EOF
evaluate
0
2x + 1
EOF

run_test "Тест 17: Пустые скобки" "ERROR: Empty parentheses" <<EOF
evaluate
0
sin()
EOF

run_test "Тест 18: Вызов функции без скобок" "ERROR: Function sin must be followed by '('" <<EOF
evaluate
1
x
0
sin x
EOF

# --- БЛОК 6: Ошибки переменных ---

run_test "Тест 19: Переменная с именем зарезервированной функции" "ERROR: Variable name cannot be a reserved function name (sin)" <<EOF
evaluate
1
sin
5
sin + 2
EOF

run_test "Тест 20: Дублирование имени переменной" "ERROR: Duplicate variable defined: x" <<EOF
evaluate
2
x x
1 2
x + 5
EOF

# Очистка временных файлов
rm -f temp_input.txt temp_output.txt

echo "======================================"
echo " Результаты: Успешно $passed из $total"
if [ "$passed" -eq "$total" ]; then
    echo " ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!"
    exit 0
else
    echo " ЕСТЬ ОШИБКИ В ТЕСТАХ!"
    exit 1
fi

//chmod +x run_tests.sh
//./run_tests.sh