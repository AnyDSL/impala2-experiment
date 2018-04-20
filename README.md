|                   | Debug             | Release           |
|-------------------|-------------------|-------------------|
| **gcc**           | [![Build1][1]][5] | [![Build2][2]][5] |
| **clang**         | [![Build3][3]][5] | [![Build4][4]][5] |

[1]: https://travis-matrix-badges.herokuapp.com/repos/AnyDSL/impala2/branches/master/1
[2]: https://travis-matrix-badges.herokuapp.com/repos/AnyDSL/impala2/branches/master/2
[3]: https://travis-matrix-badges.herokuapp.com/repos/AnyDSL/impala2/branches/master/3
[4]: https://travis-matrix-badges.herokuapp.com/repos/AnyDSL/impala2/branches/master/4
[5]: https://travis-ci.org/AnyDSL/impala2/

# Impala 2

## Building

```
git clone --recurse-submodules git@github.com:AnyDSL/impala2.git
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

```ebnf
(*pattern base*)
_p = id [":" e]                                       (*identifier pattern*)
   | "(" p "," ... "," p ")" [":" e]                  (*tuple pattern*)
   ;

(* pattern with mandatory type*)
pt = _p ":" e;

(* pattern with optional type*)
p = _p [":" e];

(* expression *)
e = id
  | "[" e | pt "," ... "," e | pt "]"                (*sigma expression*)
  | "(" [id "="] e "," ... "," [id "="] e")" [":" e] (*tuple expression*)
  | "." id                                           (*field expression*)
  | "[" e | pt ";" e "]"                             (*variadic expression*)
  | "(" e | pt ";" e")"                              (*pack expression*)
  | e "->" e                                         (*pi expression*)
  | "[" p "]" ["->" e ] e                            (*abstraction expression*)
  | e "[" e "]"                                      (*application expression*)
  | "Fn" e "->" e                                    (*Fn type expression*)
  | "fn" p ["->" e] e                                (*function expression*)
  | e e                                              (*cps call expression*)
  | "Cn" e                                           (*Cn type expression*)
  | "cn" p e                                         (*continuation expression*)
  | "if" e B ["else" B]                              (*if expression*)
  | "match" e "{" p "=>" e "," ... "," p "=>" e "}"  (*match expression*)
  | "while" e B                                      (*while expression*)
  | "for" p "in" e                                   (*for expression*)
  | B                                                (*block expression*)
  ;

(* block expression *)
B = "{" s ... s [ e ] "}"
  ;

(* statement *)
s = e ";"               (*expression statement*)
  | "let" p "=" e ";"   (*let statement*)
  ;
```

## Tips using ```git``` Submodules

This will automatically checkout the proper commit of all submodules when switching branches via ```git checkout my_branch```:
```
git config --global submodule.recurse true
```
