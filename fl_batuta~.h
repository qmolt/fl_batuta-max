/* -------------------------------------------------------------------------- */
#include "ext.h"				// standard Max include, always required
#include "ext_obex.h"			// required for new style Max object
#include "ext_dictionary.h"		// textfield
#include "jpatcher_api.h"		// includes data structures and accessor functions required by UI objects.
#include "jpatcher_syms.h"		// textfield
#include "jgraphics.h"			// includes data structures and functions for drawing.
#include "ext_boxstyle.h"		// class_attr_style_rgba_preview
#include "z_dsp.h"				
#include "common/commonsyms.c"	
#include "ext_critical.h"
//#include "ext_common.h"
//#include "ext_parameter.h"
//#include "ext_drag.h"
#include <time.h>
#include <ext_linklist.h>

/* class and symbols ---------------------------------------------------------*/
static t_class *s_fl_batuta_class;

static t_symbol *ps_elementcolor;
static t_symbol *ps_cursorcolor;
static t_symbol *ps_cursor_layer;
static t_symbol *ps_textin_layer;
static t_symbol *ps_bars;
static t_symbol *ps_cat_compas;
static t_symbol *ps_cat_canal;
static t_symbol *ps_cat_nota;
static t_symbol *ps_cat_cifra;
static t_symbol *ps_cat_goton;
static t_symbol *ps_cat_tempo;
static t_symbol *ps_subc_nota_info;
static t_symbol *ps_subc_nota_canal;
static t_symbol *ps_subc_nota_inicio;

static t_symbol *ps_acc_nuevo;
static t_symbol *ps_acc_editar;
static t_symbol *ps_acc_borrar;
static t_symbol *ps_acc_copiar;
static t_symbol *ps_acc_mover;
static t_symbol *ps_acc_info;
static t_symbol *ps_acc_human;
static t_symbol *ps_acc_quantize;
static t_symbol *ps_acc_repi;
static t_symbol *ps_acc_repf;

static t_symbol *ps_acc_read;
static t_symbol *ps_acc_write;

/* definitions ---------------------------------------------------------------*/
#define INRANGE(v,lo,hi) ((v)<=(hi)&&(v)>=(lo))
#define LADO_CUADRADO (12.0)
#define DOCE_HORAS 2073600000
#define LARGO_MAX_LINEA 1024
#define CURVE_MIN ((float)-0.98)
#define CURVE_MAX ((float)0.98)
#define NELEMS_REC 500
#define MAX_NOTAS_OUT (128)

#define BOX_SIZE_CHAR "0. 0. 600. 180."
#define BOX_TOTAL_W (600.)
#define BOX_TOTAL_H (200.)
#define BOX_INFO_H (60.)
#define BOX_TEXTF_H (20.)
#define BOX_FIJO_H (BOX_INFO_H+BOX_TEXTF_H)

/* inlets/outlets ------------------------------------------------------------*/
enum INLETS { I_ONOFF, I_NEXTBAR, NUM_INLETS };	
enum OUTLETS { O_BEATSIG, O_BARSIG, O_TEMPO, O_CIFRA, O_COMPAS, O_OUTPUT, O_BANG, NUM_OUTLETS };

/* structures --------------------------------------------------------------- */
//nota rec, nota, tempo, cifra, goto, bar, batuta obj
typedef struct _fl_nota_rec {
	t_atom *ap;
	long ac;
}fl_nota_rec;

typedef struct _fl_nota {
	t_atom *pnota;
	long cnota;
	float b_inicio;
	long canal;
	t_pt pos_ui;
}fl_nota;

typedef struct _fl_tempo {
	short type;
	long n_bar;
	float ms_inicio;
	float ms_beat;
	float ms_durvar;
	float curva;
}fl_tempo;

typedef struct _fl_cifra {
	float negras;
	long n_bar;
}fl_cifra;

typedef struct _fl_goto {
	long n_bar;
	long to_bar;
	long total_rep;
	long cont_rep;
}fl_goto;

typedef struct _fl_bar {
	t_linklist *notas;
	float cifra_ui;
	fl_tempo tempo_ui;
	fl_goto *pgoto_ui;
	short isgoto_ui;
	long total_notas;
	fl_nota **hdl_nota;
}fl_bar;

typedef struct _fl_batuta
{
	//header for UI objects
	t_pxjbox p_obj;	

	double sr;
	short onoff;

	//lectura-escritura
	long largo_texto;

	//edicion
	t_linklist *l_cifras;
	t_linklist *l_tempos;
	t_linklist *l_gotos;
	t_linklist *l_bars;

	//reproducir
	fl_cifra **cifras;
	fl_goto **gotos;
	fl_tempo **tempos;
	fl_bar **compases;
	fl_nota **notas_out;
	long total_bars;
	long total_cifras;
	long total_tempos;
	long total_gotos;
	long total_notas_out;

	long next_bar;
	short next_bar_dirty;

	short tempo_dirty;
	short cifra_dirty;
	short nota_dirty;

	//perform
	float ms_beat;
	long n_bar;
	float negras;
	long index_compas;
	long index_tempo;
	long index_cifra;
	long index_goto;
	long index_nota;
	double samps_bar;
	long total_samps;
	long samps_beat;
	long total_beat;

	//cambio de tempo
	float old_msbeat;
	float new_msbeat;
	short dtempo_busy;
	long cont_tempo;
	long durac_dtempo;
	float curva_dtempo;
	short type_dtempo;
	long delay_dtempo;

	//rec
	fl_nota_rec *rec_data;
	long index_elem_rec;
	long total_rec;
	short dirty_rec;

	//ui
	t_jrgba u_background;   // background color
	t_jrgba u_lineas;		// lines color
	t_jrgba u_cursor;		// cursor color
	t_jrgba	j_rectcolor;	// rectangle color
	t_jrgba	j_overcolor;	// rectangle over color
	t_jrgba j_textcolor;//textfield
	t_jrgba j_textbg;//textfield
	long j_overnota;		// index de la nota bajo el mouse
	long j_selnota;
	double pos_cursor_norm;
	double fasor_cursor;
	long jn_bar;
	long jn_total_bars;
	short jn_cfra_ini;
	short jn_tmpo_ini;
	short jn_nota_ini;
	fl_bar *pbar_error;
	t_atom *parsed_atoms;

	//clocks
	void *bang_clock;
	void *outmes_clock;
	void *outbar_clock;
	void *outcifra_clock;
	void *cursor_clock;
	char startclock;

	//interno
	void *outlet_mevent;
	void *outlet_ibar;
	void *outlet_bang;
	void *outlet_fcifra;
} t_fl_batuta;

/* methods ------------------------------------------------------------------- */
	//Max object
void fl_batuta_symbols_init(void);
void *fl_batuta_new(t_symbol *s, long argc, t_atom *argv);
void fl_batuta_assist(t_fl_batuta *x, void *b, long m, long a, char *s);
	//---memory
void fl_batuta_free(t_fl_batuta *x);
	//---outlets
void fl_batuta_bang(t_fl_batuta *x);
void fl_batuta_outmensaje(t_fl_batuta *x);
void fl_batuta_outcompas(t_fl_batuta *x);
void fl_batuta_outcifra(t_fl_batuta *x);
	//---console
void fl_batuta_info(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);

	//audio
void fl_batuta_dsp64(t_fl_batuta *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void fl_batuta_perform64(t_fl_batuta *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

	//UI
void fl_batuta_tick(t_fl_batuta *x);
void fl_batuta_paint(t_fl_batuta *x, t_object *patcherview);
void fl_batuta_paint_bars(t_fl_batuta *x, t_object *view, t_rect *rect);
void fl_batuta_paint_cursor(t_fl_batuta *x, t_object *view, t_rect *rect);
void fl_batuta_paint_textfield(t_fl_batuta *x, t_object *view, t_rect *rect);
void fl_batuta_mousemove(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers);
void fl_batuta_mouseleave(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers);
void fl_batuta_mousedown(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers);
//t_max_err fl_batuta_notify(t_fl_batuta *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void fl_batuta_actualizar_uibar(t_fl_batuta *x);
long fl_batuta_key(t_fl_batuta *x, t_object *patcherview, long keycode, long modifiers, long textcharacter);
long fl_batuta_keyfilter(t_fl_batuta *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter);
void fl_batuta_enter(t_fl_batuta *x);

	//edit
	//---bar
void fl_batuta_nuevo_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_add_bar(t_fl_batuta *x, long pos);
void fl_batuta_borrar_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_delete_bar(t_fl_batuta *x, long pos);
void fl_batuta_copiar_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_copy_bar(t_fl_batuta *x, long orig, long dest);
void fl_batuta_mover_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_move_bar(t_fl_batuta *x, long orig, long dest);
	//---channel
void fl_batuta_borrar_canal(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_delete_chan(t_fl_batuta *x, long bar, long chan);
void fl_batuta_copiar_canal(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_copy_chan(t_fl_batuta *x, long chan, long bar_orig, long bar_dest);
void fl_batuta_editar_canal(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_edit_chan(t_fl_batuta *x, long bar, long chan_ini, long chan_fin);
	//---note
void fl_batuta_nueva_nota(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void fl_batuta_borrar_nota(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void fl_batuta_editar_nota(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_add_note(t_fl_batuta *x, long bar, float inicio, long canal, long listac, t_atom *listav);
t_max_err do_delete_note(t_fl_batuta *x, long bar, long index_nota);
t_max_err do_edit_note_info(t_fl_batuta *x, long bar, long index_nota, long listac, t_atom *listav);
t_max_err do_edit_note_inicio(t_fl_batuta *x, long bar, long index_nota, float inicio);
t_max_err do_edit_note_canal(t_fl_batuta *x, long bar, long index_nota, long canal);
	//---measure
void fl_batuta_nueva_cifra(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_add_cifra(t_fl_batuta *x, long bar, float numer, long denom); 
void fl_batuta_borrar_cifra(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_delete_cifra(t_fl_batuta *x, long bar);
	//---tempo
void fl_batuta_nuevo_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_add_tempo(t_fl_batuta *x, short tipo, long bar, float inicio, float msbeat, float var, float cur);
void fl_batuta_borrar_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv); 
void do_delete_tempo(t_fl_batuta *x, long bar);
	//---goto
void fl_batuta_nuevo_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_add_goto(t_fl_batuta *x, long bar_orig, long bar_dest, long repet);
void fl_batuta_borrar_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_delete_goto(t_fl_batuta *x, long bar);
	//---time func

void fl_batuta_quantize_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_quantize_chan(t_fl_batuta *x, long bar, long chan, float div);
void fl_batuta_human_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void do_human_chan(t_fl_batuta *x, long bar, long chan, float d_fijo, float d_random);

	//perform
	//---preperform
void fl_batuta_actualizar(t_fl_batuta *x);
void fl_batuta_actualizar_notas(t_fl_batuta *x);
void fl_batuta_actualizar_tempos(t_fl_batuta *x);
void fl_batuta_actualizar_cifras(t_fl_batuta *x);
void fl_batuta_actualizar_gotos(t_fl_batuta *x);
	//---perform
void fl_batuta_onoff(t_fl_batuta *x, long n);
void reset_cont_goto(t_fl_batuta *x);
void fl_batuta_nextbar(t_fl_batuta *x, long n);
void fl_batuta_rec(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void fl_batuta_actualizar_rec(t_fl_batuta *x);

	//storage
void fl_batuta_read(t_fl_batuta *x, t_symbol *s);
void fl_batuta_doread(t_fl_batuta *x, t_symbol *s);
void fl_batuta_write(t_fl_batuta *x, t_symbol *s);
void fl_batuta_dowrite(t_fl_batuta *x, t_symbol *s);
void fl_batuta_writefile(t_fl_batuta *x, char *filename, short path);
void fl_batuta_openfile(t_fl_batuta *x, char *filename, short path);

	//aux
	//---UI
t_double huetorgb(double p, double q, double t);
t_jrgb hsltorgb(double h, double s, double l);
	//---text
int getlinea(char *dest, char *orig, int lim);
void my_setparse(long *ac, t_atom **av, char *line);
void my_gettext(long ac, t_atom *av, long *text_len, char **text, long flag);
	//---math
float parse_curve(float curva);
float msbeat_to_bpm(float ms_beat);
	//---comparison
long cifra_compasmenor(fl_cifra *a, fl_cifra *b);
long goto_compasmenor(fl_goto *a, fl_goto *b);
long tempo_compasmenor(fl_tempo *a, fl_tempo *b);
long nota_iniciomenor(fl_nota *a, fl_nota *b);
long cifra_mismocompas(fl_cifra *a, fl_cifra *b);
long goto_mismocompas(fl_goto *a, fl_goto *b);
long tempo_mismocompas(fl_tempo *a, fl_tempo *b);
