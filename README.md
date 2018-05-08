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
p = _p [":" e];                                      (* pattern with optional type *)
pt = _p ":" e | e;                                   (* pattern with mandatory type *)
_p = ID | tp;                                        (* pattern base *)
tp = "(" p "," ... "," p ")";                        (* tuple pattern *)

(* expressions *)
e = ID
  | "[" pt "," ... "," pt "]"                        (* sigma *)
  | "(" [ID "="] e "," ... "," [ID "="] e")" [":" e] (* tuple *)
  | "." ID                                           (* field  *)
  | "ar" "[" pt "," ... "," pt ";" e "]"             (* variadic *)
  | "pk" "(" pt "," ... "," pt ";" e ")"             (* pack *)
  | "\/" pt "->" e                                   (* abstraction type *)
  | "Fn" pt "->" e                                   (* function type *)
  | "Cn" e                                           (* continuation type *)
  | "\" tp ["->" e] e                                (* abstraction *)
  | "fn" A tp ["->" e] e                             (* function *)
  | "cn" A tp e                                      (* continuation *)
  | e "(" e ")"                                      (* cps application *)
  | e "[" e "]"                                      (* ds application *)
  | "if" e B ["else" B]                              (* if *)
  | "match" e "{" p "=>" e "," ... "," p "=>" e "}"  (* match *)
  | "while" e B                                      (* while *)
  | "for" p "in" e                                   (* for *)
  | B                                                (* block *)
  ;

A = "[" p "," ... "," p "]" | (*nothing*);           (* optional inline abstraction *)

B = "{" s ... s [ e ] "}";                           (* block expression *)

(* statement *)
s = e ";"                                            (* expression statement *)
  | "let" p "=" e ";"                                (* let statement *)
  | i                                                (* item statement *)

(* items *)
i = "fn" ID A tp ["->" e] e                          (* function item *)
  | "cn" ID A tp e                                   (* continuation item *)
  ;
```

## Tips using ```git``` Submodules

This will automatically checkout the proper commit of all submodules when switching branches via ```git checkout my_branch```:
```
git config --global submodule.recurse true
```
