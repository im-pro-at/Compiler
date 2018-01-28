#!/bin/nawk -f
/^%%$/ {
  part+=1;
  FS="#"; 
  print $0; 
  rule=1;
  next;
}
part==0
part==1 && NF>0 {
  printf "%s = %d",$1,rule;
  if ($2!="")
    printf " (%s)",$2;
  printf ";\n";
  action[rule]=$3;
  rule++;
  next;
}
part==2
END {
  if (part==1)
    print"%%"
  print"void burm_reduce(NODEPTR_TYPE bnode, int goalnt, regsp regs)";
  print"{";
  print"  int ruleNo = burm_rule (STATE_LABEL(bnode), goalnt);";
  print"  short *nts = burm_nts[ruleNo];";
  print"  NODEPTR_TYPE kids[100];";
  print"  int i;";
  print"";
  print"  if (ruleNo==0) {";
  print"    fprintf(stderr, \"tree cannot be derived from start symbol\");";
  print"    exit(1);";
  print"  }";
  print"  burm_kids (bnode, ruleNo, kids);";
  print"  for (i = 0; nts[i]; i++)";
  print"    burm_reduce (kids[i], nts[i],regs);    /* reduce kids */";
  print"";
  print"  D(\"#BURM:  %s\\n\", burm_string[ruleNo]);  /* display rule */";
  print"";
  print"  switch (ruleNo) {";
  for (i in action) {
    print "  case "i":";
    print action[i];
    print "    break;";
  }
  print"  default:    assert (0);";
  print"  }";
  print"}";
}
