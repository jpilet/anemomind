/*
 * Classic example grammar, which recognizes simple arithmetic expressions like
 * "2*(3+4)". The parser generated from this grammar then computes their value.

http://pegjs.majda.cz/online
 
 */


A = 
  "a" acc:A {acc.push(0); return acc;}
  / "a" {return [0];}
