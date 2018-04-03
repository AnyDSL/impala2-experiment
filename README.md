# Impala 2

## Building

```
git clone git@github.com:leissa/thorin2.git
cd thorin2
mkdir build
cd build
cmake ..
make
```

## Syntax

```
b ::=   x: e
    |   e

p ::=   b
    |   mut p
    |   (p, ..., p)
    |   (p; p)

e ::=
    |   [e, ..., e] |   (e, ..., e)     | . i
    |   [e; e]      |   (e; e)
    |   e -> e      |   <p> e           | e <e>
    |   Fn e -> e   |   fn p [-> e] e   | e e
    |   Cn e        |   cn p e          | e e
    |   if e B [else B]
    |   match p { p => e, ..., p => e }
    |   while e B
    |   for p in e
    |   with p in e
    |   { e; ... e; [ e ] }
```
