/*	$OpenBSD: src/lib/libcurses/parametrized.h,v 1.2 1999/01/18 19:09:04 millert Exp $	*/

/*
 * parametrized.h --- is a termcap capability parametrized?
 *
 * Note: this file is generated using parametrized.sh, do not edit by hand.
 * A value of -1 in the table means suppress both pad and % translations.
 * A value of 0 in the table means do pad but not % translations.
 * A value of 1 in the table means do both pad and % translations.
 */

static short const parametrized[] = {
0,	/*  cbt  */
0,	/*  bel  */
0,	/*  cr  */
1,	/*  csr  */
0,	/*  tbc  */
0,	/*  clear  */
0,	/*  el  */
0,	/*  ed  */
1,	/*  hpa  */
0,	/*  cmdch  */
1,	/*  cup  */
0,	/*  cud1  */
0,	/*  home  */
0,	/*  civis  */
0,	/*  cub1  */
0,	/*  mrcup  */
0,	/*  cnorm  */
0,	/*  cuf1  */
0,	/*  ll  */
0,	/*  cuu1  */
0,	/*  cvvis  */
0,	/*  dch1  */
0,	/*  dl1  */
0,	/*  dsl  */
0,	/*  hd  */
0,	/*  smacs  */
0,	/*  blink  */
0,	/*  bold  */
0,	/*  smcup  */
0,	/*  smdc  */
0,	/*  dim  */
0,	/*  smir  */
0,	/*  invis  */
0,	/*  prot  */
0,	/*  rev  */
0,	/*  smso  */
0,	/*  smul  */
1,	/*  ech  */
0,	/*  rmacs  */
0,	/*  sgr0  */
0,	/*  rmcup  */
0,	/*  rmdc  */
0,	/*  rmir  */
0,	/*  rmso  */
0,	/*  rmul  */
0,	/*  flash  */
0,	/*  ff  */
0,	/*  fsl  */
0,	/*  is1  */
0,	/*  is2  */
0,	/*  is3  */
0,	/*  if  */
0,	/*  ich1  */
0,	/*  il1  */
0,	/*  ip  */
0,	/*  kbs  */
0,	/*  ktbc  */
0,	/*  kclr  */
0,	/*  kctab  */
0,	/*  kdch1  */
0,	/*  kdl1  */
0,	/*  kcud1  */
0,	/*  krmir  */
0,	/*  kel  */
0,	/*  ked  */
0,	/*  kf0  */
0,	/*  kf1  */
0,	/*  kf10  */
0,	/*  kf2  */
0,	/*  kf3  */
0,	/*  kf4  */
0,	/*  kf5  */
0,	/*  kf6  */
0,	/*  kf7  */
0,	/*  kf8  */
0,	/*  kf9  */
0,	/*  khome  */
0,	/*  kich1  */
0,	/*  kil1  */
0,	/*  kcub1  */
0,	/*  kll  */
0,	/*  knp  */
0,	/*  kpp  */
0,	/*  kcuf1  */
0,	/*  kind  */
0,	/*  kri  */
0,	/*  khts  */
0,	/*  kcuu1  */
0,	/*  rmkx  */
0,	/*  smkx  */
0,	/*  lf0  */
0,	/*  lf1  */
0,	/*  lf10  */
0,	/*  lf2  */
0,	/*  lf3  */
0,	/*  lf4  */
0,	/*  lf5  */
0,	/*  lf6  */
0,	/*  lf7  */
0,	/*  lf8  */
0,	/*  lf9  */
0,	/*  rmm  */
0,	/*  smm  */
0,	/*  nel  */
0,	/*  pad  */
1,	/*  dch  */
1,	/*  dl  */
1,	/*  cud  */
1,	/*  ich  */
1,	/*  indn  */
1,	/*  il  */
1,	/*  cub  */
1,	/*  cuf  */
1,	/*  rin  */
1,	/*  cuu  */
1,	/*  pfkey  */
1,	/*  pfloc  */
1,	/*  pfx  */
0,	/*  mc0  */
0,	/*  mc4  */
0,	/*  mc5  */
1,	/*  rep  */
0,	/*  rs1  */
0,	/*  rs2  */
0,	/*  rs3  */
0,	/*  rf  */
0,	/*  rc  */
1,	/*  vpa  */
0,	/*  sc  */
0,	/*  ind  */
0,	/*  ri  */
1,	/*  sgr  */
0,	/*  hts  */
1,	/*  wind  */
0,	/*  ht  */
0,	/*  tsl  */
0,	/*  uc  */
0,	/*  hu  */
0,	/*  iprog  */
0,	/*  ka1  */
0,	/*  ka3  */
0,	/*  kb2  */
0,	/*  kc1  */
0,	/*  kc3  */
1,	/*  mc5p  */
0,	/*  rmp  */
-1,	/*  acsc  */
1,	/*  pln  */
0,	/*  kcbt  */
0,	/*  smxon  */
0,	/*  rmxon  */
0,	/*  smam  */
0,	/*  rmam  */
0,	/*  xonc  */
0,	/*  xoffc  */
0,	/*  enacs  */
0,	/*  smln  */
0,	/*  rmln  */
0,	/*  kbeg  */
0,	/*  kcan  */
0,	/*  kclo  */
0,	/*  kcmd  */
0,	/*  kcpy  */
0,	/*  kcrt  */
0,	/*  kend  */
0,	/*  kent  */
0,	/*  kext  */
0,	/*  kfnd  */
0,	/*  khlp  */
0,	/*  kmrk  */
0,	/*  kmsg  */
0,	/*  kmov  */
0,	/*  knxt  */
0,	/*  kopn  */
0,	/*  kopt  */
0,	/*  kprv  */
0,	/*  kprt  */
0,	/*  krdo  */
0,	/*  kref  */
0,	/*  krfr  */
0,	/*  krpl  */
0,	/*  krst  */
0,	/*  kres  */
0,	/*  ksav  */
0,	/*  kspd  */
0,	/*  kund  */
0,	/*  kBEG  */
0,	/*  kCAN  */
0,	/*  kCMD  */
0,	/*  kCPY  */
0,	/*  kCRT  */
0,	/*  kDC  */
0,	/*  kDL  */
0,	/*  kslt  */
0,	/*  kEND  */
0,	/*  kEOL  */
0,	/*  kEXT  */
0,	/*  kFND  */
1,	/*  kHLP  */
1,	/*  kHOM  */
1,	/*  kIC  */
1,	/*  kLFT  */
0,	/*  kMSG  */
0,	/*  kMOV  */
0,	/*  kNXT  */
0,	/*  kOPT  */
0,	/*  kPRV  */
0,	/*  kPRT  */
0,	/*  kRDO  */
0,	/*  kRPL  */
0,	/*  kRIT  */
0,	/*  kRES  */
0,	/*  kSAV  */
0,	/*  kSPD  */
0,	/*  kUND  */
0,	/*  rfi  */
0,	/*  kf11  */
0,	/*  kf12  */
0,	/*  kf13  */
0,	/*  kf14  */
0,	/*  kf15  */
0,	/*  kf16  */
0,	/*  kf17  */
0,	/*  kf18  */
0,	/*  kf19  */
0,	/*  kf20  */
0,	/*  kf21  */
0,	/*  kf22  */
0,	/*  kf23  */
0,	/*  kf24  */
0,	/*  kf25  */
0,	/*  kf26  */
0,	/*  kf27  */
0,	/*  kf28  */
0,	/*  kf29  */
0,	/*  kf30  */
0,	/*  kf31  */
0,	/*  kf32  */
0,	/*  kf33  */
0,	/*  kf34  */
0,	/*  kf35  */
0,	/*  kf36  */
0,	/*  kf37  */
0,	/*  kf38  */
0,	/*  kf39  */
0,	/*  kf40  */
0,	/*  kf41  */
0,	/*  kf42  */
0,	/*  kf43  */
0,	/*  kf44  */
0,	/*  kf45  */
0,	/*  kf46  */
0,	/*  kf47  */
0,	/*  kf48  */
0,	/*  kf49  */
0,	/*  kf50  */
0,	/*  kf51  */
0,	/*  kf52  */
0,	/*  kf53  */
0,	/*  kf54  */
0,	/*  kf55  */
0,	/*  kf56  */
0,	/*  kf57  */
0,	/*  kf58  */
0,	/*  kf59  */
0,	/*  kf60  */
0,	/*  kf61  */
0,	/*  kf62  */
0,	/*  kf63  */
0,	/*  el1  */
0,	/*  mgc  */
0,	/*  smgl  */
0,	/*  smgr  */
0,	/*  fln  */
1,	/*  sclk  */
1,	/*  dclk  */
0,	/*  rmclk  */
1,	/*  cwin  */
1,	/*  wingo  */
0,	/*  hup  */
1,	/*  dial  */
1,	/*  qdial  */
0,	/*  tone  */
0,	/*  pulse  */
0,	/*  hook  */
0,	/*  pause  */
0,	/*  wait  */
1,	/*  u0  */
1,	/*  u1  */
1,	/*  u2  */
1,	/*  u3  */
1,	/*  u4  */
1,	/*  u5  */
1,	/*  u6  */
1,	/*  u7  */
1,	/*  u8  */
1,	/*  u9  */
0,	/*  op  */
0,	/*  oc  */
1,	/*  initc  */
1,	/*  initp  */
1,	/*  scp  */
1,	/*  setf  */
1,	/*  setb  */
0,	/*  cpi  */
0,	/*  lpi  */
0,	/*  chr  */
0,	/*  cvr  */
0,	/*  defc  */
0,	/*  swidm  */
0,	/*  sdrfq  */
0,	/*  sitm  */
0,	/*  slm  */
0,	/*  smicm  */
0,	/*  snlq  */
0,	/*  snrmq  */
0,	/*  sshm  */
0,	/*  ssubm  */
0,	/*  ssupm  */
0,	/*  sum  */
0,	/*  rwidm  */
0,	/*  ritm  */
0,	/*  rlm  */
0,	/*  rmicm  */
0,	/*  rshm  */
0,	/*  rsubm  */
0,	/*  rsupm  */
0,	/*  rum  */
0,	/*  mhpa  */
0,	/*  mcud1  */
0,	/*  mcub1  */
0,	/*  mcuf1  */
0,	/*  mvpa  */
0,	/*  mcuu1  */
0,	/*  porder  */
0,	/*  mcud  */
0,	/*  mcub  */
0,	/*  mcuf  */
0,	/*  mcuu  */
0,	/*  scs  */
0,	/*  smgb  */
1,	/*  smgbp  */
1,	/*  smglp  */
1,	/*  smgrp  */
0,	/*  smgt  */
1,	/*  smgtp  */
0,	/*  sbim  */
0,	/*  scsd  */
0,	/*  rbim  */
0,	/*  rcsd  */
0,	/*  subcs  */
0,	/*  supcs  */
0,	/*  docr  */
0,	/*  zerom  */
0,	/*  csnm  */
0,	/*  kmous  */
0,	/*  minfo  */
0,	/*  reqmp  */
0,	/*  getm  */
0,	/*  setaf  */
0,	/*  setab  */
1,	/*  pfxl  */
0,	/*  devt  */
0,	/*  csin  */
0,	/*  s0ds  */
0,	/*  s1ds  */
0,	/*  s2ds  */
0,	/*  s3ds  */
1,	/*  smglr  */
1,	/*  smgtb  */
1,	/*  birep  */
0,	/*  binel  */
0,	/*  bicr  */
1,	/*  colornm  */
0,	/*  defbi  */
0,	/*  endbi  */
1,	/*  setcolor  */
1,	/*  slines  */
0,	/*  dispc  */
0,	/*  smpch  */
0,	/*  rmpch  */
0,	/*  smsc  */
0,	/*  rmsc  */
0,	/*  pctrm  */
0,	/*  scesc  */
0,	/*  scesa  */
0,	/*  ehhlm  */
0,	/*  elhlm  */
0,	/*  elohlm  */
0,	/*  erhlm  */
0,	/*  ethlm  */
0,	/*  evhlm  */
1,	/*  sgr1  */
1,	/*  slength  */
0,	/*  OTi2  */
0,	/*  OTrs  */
0,	/*  OTnl  */
0,	/*  OTbc  */
0,	/*  OTko  */
0,	/*  OTma  */
-1,	/*  OTG2  */
-1,	/*  OTG3  */
-1,	/*  OTG1  */
-1,	/*  OTG4  */
-1,	/*  OTGR  */
-1,	/*  OTGL  */
-1,	/*  OTGU  */
-1,	/*  OTGD  */
-1,	/*  OTGH  */
-1,	/*  OTGV  */
-1,	/*  OTGC  */
0,	/*  meml  */
0,	/*  memu  */
1,	/*  pln  */
0,	/*  smln  */
0,	/*  rmln  */
0,	/*  kf11  */
0,	/*  kf12  */
0,	/*  kf13  */
0,	/*  kf14  */
0,	/*  kf15  */
0,	/*  kf16  */
0,	/*  kf17  */
0,	/*  kf18  */
0,	/*  kf19  */
0,	/*  kf20  */
0,	/*  kf21  */
0,	/*  kf22  */
0,	/*  kf23  */
0,	/*  kf24  */
0,	/*  kf25  */
0,	/*  kf26  */
0,	/*  kf27  */
0,	/*  kf28  */
0,	/*  kf29  */
0,	/*  kf30  */
0,	/*  kf31  */
0,	/*  kf32  */
0,	/*  kf33  */
0,	/*  kf34  */
0,	/*  kf35  */
0,	/*  kf36  */
0,	/*  kf37  */
0,	/*  kf38  */
0,	/*  kf39  */
0,	/*  kf40  */
0,	/*  kf41  */
0,	/*  kf42  */
0,	/*  kf43  */
0,	/*  kf44  */
0,	/*  kf45  */
0,	/*  kf46  */
0,	/*  kf47  */
0,	/*  kf48  */
0,	/*  kf49  */
0,	/*  kf50  */
0,	/*  kf51  */
0,	/*  kf52  */
0,	/*  kf53  */
0,	/*  kf54  */
0,	/*  kf55  */
0,	/*  kf56  */
0,	/*  kf57  */
0,	/*  kf58  */
0,	/*  kf59  */
0,	/*  kf60  */
0,	/*  kf61  */
0,	/*  kf62  */
0,	/*  kf63  */
0,	/*  box1  */
0,	/*  box2  */
0,	/*  batt1  */
0,	/*  batt2  */
0,	/*  colb0  */
0,	/*  colb1  */
0,	/*  colb2  */
0,	/*  colb3  */
0,	/*  colb4  */
0,	/*  colb5  */
0,	/*  colb6  */
0,	/*  colb7  */
0,	/*  colf0  */
0,	/*  colf1  */
0,	/*  colf2  */
0,	/*  colf3  */
0,	/*  colf4  */
0,	/*  colf5  */
0,	/*  colf6  */
0,	/*  colf7  */
0,	/*  font0  */
0,	/*  font1  */
0,	/*  font2  */
0,	/*  font3  */
0,	/*  font4  */
0,	/*  font5  */
0,	/*  font6  */
0,	/*  font7  */
0,	/*  kbtab  */
0,	/*  kdo  */
0,	/*  kcmd  */
0,	/*  kcpn  */
0,	/*  kend  */
0,	/*  khlp  */
0,	/*  knl  */
0,	/*  knpn  */
0,	/*  kppn  */
0,	/*  kppn  */
0,	/*  kquit  */
0,	/*  ksel  */
0,	/*  kscl  */
0,	/*  kscr  */
0,	/*  ktab  */
0,	/*  kmpf1  */
0,	/*  kmpt1  */
0,	/*  kmpf2  */
0,	/*  kmpt2  */
0,	/*  kmpf3  */
0,	/*  kmpt3  */
0,	/*  kmpf4  */
0,	/*  kmpt4  */
0,	/*  kmpf5  */
0,	/*  kmpt5  */
0,	/*  apstr  */
0,	/*  kmpf6  */
0,	/*  kmpt6  */
0,	/*  kmpf7  */
0,	/*  kmpt7  */
0,	/*  kmpf8  */
0,	/*  kmpt8  */
0,	/*  kmpf9  */
0,	/*  kmpt9  */
0,	/*  ksf1  */
0,	/*  ksf2  */
0,	/*  ksf3  */
0,	/*  ksf4  */
0,	/*  ksf5  */
0,	/*  ksf6  */
0,	/*  ksf7  */
0,	/*  ksf8  */
0,	/*  ksf9  */
0,	/*  ksf10  */
0,	/*  kf11  */
0,	/*  kf12  */
0,	/*  kf13  */
0,	/*  kf14  */
0,	/*  kf15  */
0,	/*  kf16  */
0,	/*  kf17  */
0,	/*  kf18  */
0,	/*  kf19  */
0,	/*  kf20  */
0,	/*  kf21  */
0,	/*  kf22  */
0,	/*  kf23  */
0,	/*  kf24  */
0,	/*  kf25  */
0,	/*  kf26  */
0,	/*  kf26  */
0,	/*  kf28  */
0,	/*  kf29  */
0,	/*  kf30  */
0,	/*  kf31  */
0,	/*  kf31  */
0,	/*  kf33  */
0,	/*  kf34  */
0,	/*  kf35  */
0,	/*  kf36  */
0,	/*  kf37  */
0,	/*  kf38  */
0,	/*  kf39  */
0,	/*  kf40  */
0,	/*  kf41  */
0,	/*  kf42  */
0,	/*  kf43  */
0,	/*  kf44  */
0,	/*  kf45  */
0,	/*  kf46  */
0,	/*  kf47  */
0,	/*  kf48  */
0,	/*  kf49  */
0,	/*  kf50  */
0,	/*  kf51  */
0,	/*  kf52  */
0,	/*  kf53  */
0,	/*  kf54  */
0,	/*  kf55  */
0,	/*  kf56  */
0,	/*  kf57  */
0,	/*  kf58  */
0,	/*  kf59  */
0,	/*  kf60  */
0,	/*  kf61  */
0,	/*  kf62  */
0,	/*  kf63  */
0,	/*  kact  */
0,	/*  topl  */
0,	/*  btml  */
0,	/*  rvert  */
0,	/*  lvert  */
} /* 599 entries */;

