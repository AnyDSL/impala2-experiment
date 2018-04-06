|                   | Debug             | Release           |
|-------------------|-------------------|-------------------|
| **gcc**           | [![Build1][1]][5] | [![Build2][2]][5] |
| **clang**         | [![Build3][3]][5] | [![Build4][4]][5] |

[1]: https://travis-matrix-badges.herokuapp.com/repos/leissa/impala2/branches/master/1
[2]: https://travis-matrix-badges.herokuapp.com/repos/leissa/impala2/branches/master/2
[3]: https://travis-matrix-badges.herokuapp.com/repos/leissa/impala2/branches/master/3
[4]: https://travis-matrix-badges.herokuapp.com/repos/leissa/impala2/branches/master/4
[5]: https://travis-ci.org/leissa/impala2/

# Impala 2

## Building

```
git clone --recurse-submodules git@github.com:leissa/impala2.git
cd impala2
mkdir build
cd build
cmake ..
make
```

If you are a developer, you might want to use this:
```
git config --global url.ssh://git@github.com/.insteadOf https://github.com/
```
This will use SSH instead of HTTPS and will grant you push access for the submodules if applicable.

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
