// org 0x0000
B0000:
goto B0034;
// org 0x0023
B0023:
   if (TI) { TI = 0; goto B002a; }
   if (RI) { RI = 0; goto B002f; }
clri(); return;
B002a:
   R0 = 0x7d;
   C0051();
clri(); return;
B002f:
   R0 = 0x7c;
   C0051();
clri(); return;
B0034:
   D73 = 0x74;
   SP = 0x7d;
   DPTR = 0x0067;
   push(DPL);
   push(DPH);
   B0144();
B0043:
   PCON |= 0x01;
goto B0043;
C0048:
   *R0 = SP;
   D73--;
   R0 = D73;
   SP = *R0;
return;
C0051:
   R1 = D73;
   D73++;
   *R1 = SP;
   SP = *R0;
   *R0 = 0x7f;
return;
B005c:
   R1 = D73;
   D73++;
   *R1 = SP;
   R0--;
   SP = R0;
   B006e();
B0067:
   D73--;
   R0 = D73;
   SP = *R0;
return;
B006e:
   push(DPL);
   push(DPH);
return;
B0073:
   SCON = 0xc0;
   PS = 0;
   RCLK = 0;
   TCLK = 0;
   A = TMOD;
   A &= 0x0f;
   C:A += 0x20;
   TMOD = A;
   TH1 = 0xff;
   PCON |= 0x80;
   TR1 = 1;
   TB8 = 0;
   RI = 0;
   TI = 0;
   ES = 1;
return;
B0095:
   T0 = 0;
   SM2 = 1;
   REN = 1;
return;
B009c:
   REN = 0;
   T0 = 1;
return;
C00a1:
   R0 = 0x7c;
   C0048();
   A = SBUF;
   if (!SM2) goto B00b1;
   C = (A < 0x3a); if (A != 0x3a) goto B00af;
   SM2 = 0;
B00af:
goto C00a1;
B00b1:
return;
C00b2:
   A = 0x0d;
   G00b8();
   A = 0x0a;
G00b8:
   SBUF = A;
   R0 = 0x7d;
   C0048();
return;
C00bf:
   A &= 0x0f;
   C = (A < 0x0a); if (A != 0x0a) goto B00c4;
B00c4:
   if (C) goto B00cb;
   C:A += 0x57;
   G00b8();
return;
B00cb:
   C:A += 0x30;
   G00b8();
return;
L00d0:
   A <-> R3;
   C:A += R3;
   A <-> R3;
   push(A);
   A = rot(A, 4);
   C00bf();
   A = pop();
   C00bf();
return;
B00dd:
   A = 0x3a;
   G00b8();
   R3 = 0x00;
   A = 0x00;
   L00d0();
   A = DPH;
   L00d0();
   A = DPL;
   L00d0();
   A = 0x01;
   L00d0();
   A = R3;
   A = ~A;
   L00d0();
   C00b2();
return;
B00fa:
   A = 0x3a;
   G00b8();
   R3 = 0x00;
   A = 0x10;
   L00d0();
   A = DPH;
   L00d0();
   A = DPL;
   L00d0();
   A = 0x00;
   L00d0();
   R2 = 0x10;
B0112:
   A = 0;
   A = DPTR[A];
   DPTR++;
   L00d0();
   if (--R2 != 0) goto B0112;
   A = R3;
   A = ~A;
   L00d0();
   C00b2();
return;
B0120:
   DPTR = 0x0000;
B0123:
   A = DPH;
   C = (A < 0x20); if (A != 0x20) goto B0128;
B0128:
   if (!C) goto B012e;
   B00fa();
goto B0123;
B012e:
   B00dd();
return;
B0131:
   B0073();
C0133:
   B0095();
   C00a1();
   C = (A < 0x73); if (A != 0x73) goto B013e;
   B0120();
goto C0133;
B013e:
   B009c();
   G00b8();
goto C0133;
B0144:
   R0 = 0x90;
   DPTR = 0x0131;
   B005c();
   EA = 1;
return;
