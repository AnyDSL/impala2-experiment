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
(* pattern base *)
_p = id [":" e]                                       (* identifier *)
   | "(" p "," ... "," p ")" [":" e]                  (* tuple *)
   ;

(* pattern with mandatory type or expression*)
pe = _p ":" e | e;

(* pattern with optional type*)
p = _p [":" e];

(* expressions *)
e = id
  | "[" pe "," ... "," pe "]"                        (* sigma *)
  | "(" [id "="] e "," ... "," [id "="] e")" [":" e] (* tuple *)
  | "." id                                           (* field  *)
  | "ar" "[" pe ";" e "]"                            (* variadic *)
  | "pk" "(" pe ";" e ")"                            (* pack *)
  | "/\" pe "->" e                                   (* abstraction type *)
  | "Fn" pe "->" e                                   (* function type *)
  | "Cn" e                                           (* continuation type *)
  | "\"  p ["->" e] e                                (* abstraction *)
  | "fn" p ["->" e] e                                (* function *)
  | "cn" p e                                         (* continuation *)
  | e e                                              (* cps call *)
  | e "[" e "]"                                      (* application *)
  | "if" e B ["else" B]                              (* if *)
  | "match" e "{" p "=>" e "," ... "," p "=>" e "}"  (* match *)
  | "while" e B                                      (* while *)
  | "for" p "in" e                                   (* for *)
  | B                                                (* block *)
  ;

(* block expression *)
B = "{" s ... s [ e ] "}"
  ;

(* statement *)
s = e ";"               (*expression statement *)
  | "let" p "=" e ";"   (*let statement *)
  ;
```

## Tips using ```git``` Submodules

This will automatically checkout the proper commit of all submodules when switching branches via ```git checkout my_branch```:
```
git config --global submodule.recurse true
```
