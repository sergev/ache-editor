#ifdef T_tcap
extern S_term t_tcap;
#else
#include "sterm.h"
#include "e.up.h"
/****************************************/
/**** 0 = terminal from termcap ****/
extern char *tgoto ();

#undef UP
#undef HO

char *BC;
char *GE;
char *ND;
char *DO;
char *NL;
char *UP;
char *CR;
char *CL;
char *CD;
char *HO;
char *CM;
char *TI;
char *TE;
char PC;
char *AL;
char *DL;
char *CE;
char *IC;
char *DC;
char *KS;
char *KE;
char *ME;
char *LL;
char *CV;
char *CH;
char *SO;
char *SE;
char *MB;
char *MD;
char *MH;
char *MR;
char *AS;
char *AE;
char *US;
char *UE;

pch(ch)
char ch;
{
#ifdef pdp11 /* IPK Demos fix, only for pdp11 */
    if (isuppercase)
	/* only for brain damage KOI8 */
	ch = (ch >= 'A' && ch <= 'Z') ? (ch | 040) :
	     ((ch > 0137 && ch < 0177) ? (ch | 0140) : ch);
#endif
    putchar (ch);
}

lt_tcap () { tputs(BC, 0, pch); }
rt_tcap () { tputs(ND, 0, pch); }
dn_tcap () { tputs(DO, 1, pch); }
up_tcap () { tputs(UP, 1, pch); }
cr_tcap () { tputs(CR, 0, pch); }
ll_tcap () { tputs(LL, 0, pch); }
so_tcap () { tputs(SO, 0, pch); }
se_tcap () { tputs(SE, 0, pch); }
me_tcap () { tputs(ME, 0, pch); }
mb_tcap () { tputs(MB, 0, pch); }
md_tcap () { tputs(MD, 0, pch); }
mh_tcap () { tputs(MH, 0, pch); }
mr_tcap () { tputs(MR, 0, pch); }
as_tcap () { tputs(AS, 0, pch); }
ae_tcap () { tputs(AE, 0, pch); }
us_tcap () { tputs(US, 0, pch); }
ue_tcap () { tputs(UE, 0, pch); }
ge_tcap () {
    if (GE) {
    /* if your termcap entry for GE not included ME...*/
	if (ME)
	    tputs (ME, 0, pch);
	tputs(GE, 0, pch);
	attributes = IA_NORMAL;
    }
}
nl_tcap () { tputs(CR, 0, pch); tputs(DO, 1, pch); }
clr_tcap () { tputs(CL, term.tt_height, pch); }
clr1_tcap () { punt_tcap (); tputs(CD, term.tt_height, pch); }
hm_tcap () { tputs(HO, 0, pch); }
bsp_tcap () { tputs(BC, 0, pch); putchar(' '); tputs(BC, 0, pch); }
addr_tcap (lin, col) { tputs(tgoto(CM, col, lin), 0, pch); }
cad_tcap (col) { tputs(tgoto(CH, 0, col), 0, pch); }
lad_tcap (lin) { tputs(tgoto(CV, 0, lin), lin, pch); }
/* ARGSUSED */
int il_tcap (num) { tputs(AL, 1, pch); return 1; }
/* ARGSUSED */
int ic_tcap (num) { tputs(IC, 0, pch); return 1; }
/* ARGSUSED */
int dc_tcap (num) { tputs(DC, 0, pch); return 1; }
/* ARGSUSED */
int dl_tcap (num) { tputs(DL, 1, pch); return 1; }
int cle_tcap () { tputs(CE, term.tt_height, pch); return 1; }
int eos_tcap () { tputs(CD, term.tt_height, pch); return 1; }
ini1_tcap () { tputs(TI, 0, pch); }
kbi_tcap () { tputs(KS, 0, pch); }
end_tcap () { tputs(TE, 0, pch); }
kbe_tcap () { tputs(KE, 0, pch); }
punt_tcap () { tputs(tgoto (CM, icol, ilin), 0, pch); }
extern int vscset_tcap (), vscend_tcap (), scrdn_tcap (), scrup_tcap ();
extern xlate_tcap ();

S_term t_tcap = {
/* tt_ini0    */    nop,
/* tt_ini1    */    ini1_tcap,
/* tt_end     */    end_tcap,
/* tt_left    */    lt_tcap,
/* tt_right   */    rt_tcap,
/* tt_dn      */    dn_tcap,
/* tt_up      */    up_tcap,
/* tt_cret    */    cr_tcap,
/* tt_nl      */    nl_tcap,
/* tt_clear   */    clr_tcap,
/* tt_home    */    hm_tcap,
/* tt_bsp     */    bsp_tcap,
/* tt_addr    */    addr_tcap,
/* tt_lad     */    lad_tcap,
/* tt_cad     */    cad_tcap,
/* tt_xlate   */    xlate_tcap,
/* tt_insline */    il_tcap,
/* tt_delline */    dl_tcap,
/* tt_inschar */    ic_tcap,
/* tt_delchar */    dc_tcap,
/* tt_clrel  */     cle_tcap,
/* tt_clres  */     eos_tcap,
/* tt_vscset  */    vscset_tcap,
/* tt_vscend  */    vscend_tcap,
/* tt_scrup   */    scrup_tcap,
/* tt_scrdn   */    scrdn_tcap,
/* tt_deflwin */    (int (*) ()) 0,
/* tt_erase   */    (int (*) ()) 0,
/* tt_ll      */    ll_tcap,
/* tt_mexit   */    me_tcap,
/* tt_gexit   */    ge_tcap,
/* tt_so      */    so_tcap,
/* tt_se      */    se_tcap,
/* tt_mb      */    mb_tcap,
/* tt_md      */    md_tcap,
/* tt_mh      */    mh_tcap,
/* tt_mr      */    mr_tcap,
/* tt_as      */    as_tcap,
/* tt_ae      */    ae_tcap,
/* tt_us      */    us_tcap,
/* tt_ue      */    ue_tcap,
/* tt_da      */    0,
/* tt_db      */    0,
/* tt_nleft   */    0,
/* tt_nright  */    0,
/* tt_ndn     */    0,
/* tt_nup     */    0,
/* tt_nnl     */    0,
/* tt_nbsp    */    0,
/* tt_naddr   */    0,
/* tt_nlad    */    0,
/* tt_ncad    */    0,
/* tt_wl      */    1,
/* tt_cwr     */    1,
/* tt_pwr     */    1,
/* tt_axis    */    0,
/* tt_prtok   */    YES,
/* tt_width   */    80,
/* tt_height  */    24
};
#endif
