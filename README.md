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
b ::=   ID: e
    |   e

pt ::=  p: t
    |   p

p ::=   ID
    |   mut pt
    |   (pt, ..., pt)

e ::=   ID
    |   [b, ..., b] |   (e, ..., e)     | . i
    |   [b; e]      |   (b; e)
    |   e -> e      |   <p> e           | e <e>
    |   Fn e -> e   |   fn pt [-> e] e  | e e
    |   Cn e        |   cn pt e         | e e
    |   if e B [else B]
    |   match e { p => e, ..., p => e }
    |   while e B
    |   for pt in e
    |   B
    |   NUM
    |   NUM ID

B ::=  { s ... s [ e ] }

s ::= e;
    | let pt = e;
```
