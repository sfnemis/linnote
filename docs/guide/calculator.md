# Calculator

LinNote's built-in calculator evaluates expressions in real-time. Works in any mode, but Calculator mode (`calc`) is optimized for math.

## Basic Usage

Type any expression. Add `=` at the end to see the result:

```
2 + 2 =
// Result: 4

100 / 4 =
// Result: 25
```

## Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3 = 8` |
| `-` | Subtraction | `10 - 4 = 6` |
| `*` | Multiplication | `6 * 7 = 42` |
| `/` | Division | `15 / 3 = 5` |
| `^` | Power | `2^8 = 256` |
| `%` | Modulo | `17 % 5 = 2` |
| `()` | Parentheses | `(2 + 3) * 4 = 20` |

## Percentages

```
100 + 18% =
// Result: 118

500 - 20% =
// Result: 400

what is 15% of 200 =
// Result: 30

25 is what percent of 100 =
// Result: 25%
```

## Variables

Store values for reuse:

```
price = 100
quantity = 5
discount = 10

subtotal = price * quantity =
// Result: 500

discount_amount = subtotal * discount / 100 =
// Result: 50

total = subtotal - discount_amount =
// Result: 450
```

Variables persist within the same note.

## Functions

| Function | Description | Example |
|----------|-------------|---------|
| `sqrt(x)` | Square root | `sqrt(144) = 12` |
| `abs(x)` | Absolute value | `abs(-5) = 5` |
| `sin(x)` | Sine (radians) | `sin(0) = 0` |
| `cos(x)` | Cosine | `cos(0) = 1` |
| `tan(x)` | Tangent | `tan(0) = 0` |
| `log(x)` | Natural log | `log(e) = 1` |
| `log10(x)` | Log base 10 | `log10(100) = 2` |
| `exp(x)` | e^x | `exp(1) = 2.718...` |
| `floor(x)` | Round down | `floor(3.7) = 3` |
| `ceil(x)` | Round up | `ceil(3.2) = 4` |
| `round(x)` | Round nearest | `round(3.5) = 4` |
| `min(a,b)` | Minimum | `min(5, 3) = 3` |
| `max(a,b)` | Maximum | `max(5, 3) = 5` |

## Constants

| Constant | Value |
|----------|-------|
| `pi` | 3.14159265359 |
| `e` | 2.71828182846 |

## Text Analysis Commands

Analyze numbers in your note using keywords:

### sum

Adds all numbers in the current note:

```
Groceries:
Milk 4.50
Bread 2.30
Eggs 6.00
Butter 3.20

sum
```
**Result:** `Sum: 16.00`

### avg

Calculates average of all numbers:

```
Test Scores:
85
92
78
88
95

avg
```
**Result:** `Average: 87.6`

### count

Counts how many numbers are in the note:

```
Random data:
apple 5
banana 3
cherry 8
date 2

count
```
**Result:** `Count: 4`

## Practical Examples

### Tip Calculator
```
bill = 85.50
tip_percent = 18
people = 4

tip = bill * tip_percent / 100 =
// Result: 15.39

total = bill + tip =
// Result: 100.89

per_person = total / people =
// Result: 25.22
```

### Loan Payment
```
principal = 250000
annual_rate = 6.5
years = 30

monthly_rate = annual_rate / 100 / 12 =
payments = years * 12 =

// Monthly payment formula simplified
monthly = principal * monthly_rate =
// Result: ~1,580
```

### Area Calculation
```
// Circle area
radius = 5
area = pi * radius^2 =
// Result: 78.54

// Rectangle
width = 12
height = 8
rect_area = width * height =
// Result: 96
```
