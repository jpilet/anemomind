n25 = acc:(n0 / n1)+ {acc.unshift(25); return acc;}



n0 =
"n0" acc:n0 {acc[1]++; return acc;}
/ "n0" {return [0, 1];}
n1 =
"n1" acc:n1 {acc[1]++; return acc;}
/ "n1" {return [1, 1];}
n2 =
"n2" acc:n2 {acc[1]++; return acc;}
/ "n2" {return [2, 1];}
n3 =
"n3" acc:n3 {acc[1]++; return acc;}
/ "n3" {return [3, 1];}
n4 =
"n4" acc:n4 {acc[1]++; return acc;}
/ "n4" {return [4, 1];}
n5 =
"n5" acc:n5 {acc[1]++; return acc;}
/ "n5" {return [5, 1];}
