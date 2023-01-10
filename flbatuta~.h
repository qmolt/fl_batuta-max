/* -------------------------------------------------------------------------- */
#include "ext.h"				// standard Max include, always required
#include "ext_obex.h"			// required for new style Max object
//#include "ext_dictionary.h"		// textfield
#include "jpatcher_api.h"		// includes data structures and accessor functions required by UI objects.
//#include "jpatcher_syms.h"		// textfield
#include "jgraphics.h"			// includes data structures and functions for drawing.
#include "ext_boxstyle.h"		// class_attr_style_rgba_preview
#include "z_dsp.h"				
#include "ext_critical.h"
//#include "ext_common.h"
//#include "ext_parameter.h"
//#include "ext_drag.h"
#include <time.h>
#include <ext_linklist.h>

#include "json.h"

/* globals and statics ---------------------------------------------------------*/
static t_class *s_fl_batuta_class;

/* definitions ---------------------------------------------------------------*/
#define INRANGE(v,lo,hi) ((v)<=(hi)&&(v)>=(lo))
#define NOTE_W 8
#define LARGO_MAX_LINEA 1024
#define CURVE_MIN ((float)-0.98)
#define CURVE_MAX ((float)0.98)
#define NELEMS_REC 500
#define MAX_NOTES_OUT (128)

#define TEMPO_MIN (100.f)

#define COLOR_MULTIPLIER 53
#define ALPHA_NOTE_ON (0.9)
#define ALPHA_NOTE_OFF (0.4)

#define BOX_SIZE_CHAR "0. 0. 600. 180."
#define BOX_TOTAL_W (600.)
#define BOX_TOTAL_H (200.)
#define BOX_TEXTF_H (20.)
#define BOX_INFO_H (60.)
#define BOX_FIJO_H (BOX_INFO_H+BOX_TEXTF_H)

#define MAX_BUF_LEN 512

/* inlets/outlets ------------------------------------------------------------*/
enum INLETS { I_ONOFF, I_NEXTBAR, NUM_INLETS };	
enum OUTLETS { O_BEATSIG, O_BARSIG, O_TEMPO, O_CIFRA, O_COMPAS, O_OUTPUT, O_BANG, NUM_OUTLETS };

/* structures --------------------------------------------------------------- */
//nota rec, nota, tempo, cifra, goto, bar, batuta obj
typedef struct _fl_note_rec {
	t_atom *ap;
	long ac;
	long bar;
	float b_start;
}fl_note_rec;

typedef struct _fl_note {
	t_atom *pnota;
	long cnota;
	float b_inicio;
	long canal;
	t_pt pos_ui;
}fl_note;

typedef struct _fl_tempo {
	long n_bar;
	float ms_inicio;
	float ms_beat;
	float ms_durvar;
	float curva;
	float powval;
}fl_tempo;

typedef struct _fl_tsign {
	float beats;
	long n_bar;
}fl_tsign;

typedef struct _fl_goto {
	long n_bar;
	long to_bar;
	long total_rep;
	long cont_rep;
}fl_goto;

typedef struct _fl_bar {
	t_linklist *notas;
	fl_tsign *psignat_ui;
	fl_tempo *ptempo_ui;
	fl_goto *pgoto_ui;
	short isgoto_ui;
	long total_notas;
	fl_note **hdl_nota;
}fl_bar;

typedef struct _fl_batuta
{
	//header for UI objects
	t_pxjbox p_obj;	

	double sr;
	char isplaying;
	char isloading;
	char isediting;

	//lectura-escritura
	long largo_texto;
	long max_buf_len;

	//edicion
	t_linklist *l_tsigns;
	t_linklist *l_tempos;
	t_linklist *l_gotos;
	t_linklist *l_bars;

	//reproducir
	fl_tsign **tsigns;
	fl_goto **gotos;
	fl_tempo **tempos;
	fl_bar **bars;
	fl_note **notes_out;
	long total_bars;
	long total_tsigns;
	long total_tempos;
	long total_gotos;
	long total_notes_out;

	long next_bar;
	short next_bar_dirty;

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
	long delay_dtempo;

	//rec
	fl_note_rec *rec_data;
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
	char jn_show_all_notes;
	long jn_chan_min;
	long jn_chan_max;
	long j_selnota;
	char *text_info;
	long text_size;
	double pos_cursor_norm;
	double fasor_cursor;
	long jn_bar;
	long jn_total_bars;
	long jn_total_tsigns;
	long jn_total_tempos;
	long jn_total_gotos;
	t_atom *parsed_atoms;
	fl_bar error_bar;
	fl_tempo error_tempo;
	fl_tsign error_tsign;
	fl_goto error_goto;

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
void fl_batuta_show_range(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void fl_batuta_sel_note(t_fl_batuta *x, t_symbol *s, long  argc, t_atom *argv);
t_max_err fl_batuta_is_note_index(t_fl_batuta *x, long *note_index, long bar, long chan, long index_in_chan);
void fl_batuta_paint(t_fl_batuta *x, t_object *patcherview);
void fl_batuta_paint_bars(t_fl_batuta *x, t_object *view, t_rect *rect);
void fl_batuta_paint_notes(t_fl_batuta *x, t_object *view, t_rect *rect);
void fl_batuta_paint_info(t_fl_batuta *x, t_object *view, t_rect *rect);
void fl_batuta_paint_cursor(t_fl_batuta *x, t_object *view, t_rect *rect);
void fl_batuta_paint_textfield(t_fl_batuta *x, t_object *view, t_rect *rect);
//void fl_batuta_mousemove(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers);
//void fl_batuta_mouseleave(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers);
//void fl_batuta_mousedown(t_fl_batuta *x, t_object *patcherview, t_pt pt, long modifiers);
//t_max_err fl_batuta_notify(t_fl_batuta *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
long fl_batuta_key(t_fl_batuta *x, t_object *patcherview, long keycode, long modifiers, long textcharacter);
long fl_batuta_keyfilter(t_fl_batuta *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter);
void fl_batuta_enter(t_fl_batuta *x);

	//edit
	//---bar
void fl_batuta_new_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_add_bar(t_fl_batuta *x, long pos);
void fl_batuta_delete_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_delete_bar(t_fl_batuta *x, long pos);
void fl_batuta_copy_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_copy_bar(t_fl_batuta *x, long orig, long dest);
void fl_batuta_move_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_move_bar(t_fl_batuta *x, long orig, long dest);
	//---channel
void fl_batuta_delete_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_delete_chan(t_fl_batuta *x, long bar, long chan);
void fl_batuta_copy_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_copy_chan(t_fl_batuta *x, long chan, long bar_orig, long bar_dest);
void fl_batuta_edit_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_edit_chan(t_fl_batuta *x, long bar, long chan_ini, long chan_fin);
	//---note
t_max_err do_find_note_index(t_fl_batuta *x, long *note_index, long bar, long chan, long note_in_chan);
void fl_batuta_new_note(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_add_note(t_fl_batuta *x, long bar, float inicio, long canal, long listac, t_atom *listav, short prependchan);
void fl_batuta_delete_note(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_delete_note(t_fl_batuta *x, long bar, long index_nota);
void fl_batuta_edit_note(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_edit_note_info(t_fl_batuta *x, long bar, long index_nota, long listac, t_atom *listav);
t_max_err do_edit_note_start(t_fl_batuta *x, long bar, long index_nota, float inicio);
t_max_err do_edit_note_chan(t_fl_batuta *x, long bar, long index_nota, long canal);
	//---signature
void fl_batuta_new_signature(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_add_signature(t_fl_batuta *x, long bar, float beats);
void fl_batuta_delete_signature(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_delete_signature(t_fl_batuta *x, long bar);
	//---tempo
void fl_batuta_new_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_add_tempo(t_fl_batuta *x, long bar, float inicio, float msbeat, float var, float cur);
void fl_batuta_delete_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv); 
t_max_err do_delete_tempo(t_fl_batuta *x, long bar);
	//---goto
void fl_batuta_new_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_add_goto(t_fl_batuta *x, long bar_orig, long bar_dest, long repet);
void fl_batuta_delete_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_delete_goto(t_fl_batuta *x, long bar);
	//---time func
void fl_batuta_quantize_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_quantize_chan(t_fl_batuta *x, long bar, long chan, float div);
void fl_batuta_human_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
t_max_err do_human_chan(t_fl_batuta *x, long bar, long chan, float d_fijo, float d_random);

	//perform
	//---preperform
t_max_err fl_batuta_update_notes_onebar(t_fl_batuta *x, long n);
t_max_err fl_batuta_update_notes(t_fl_batuta *x);
t_max_err fl_batuta_update_tempos(t_fl_batuta *x);
t_max_err fl_batuta_update_signatures(t_fl_batuta *x);
t_max_err fl_batuta_update_gotos(t_fl_batuta *x);
t_max_err fl_batuta_update_uitempo(t_fl_batuta *x);
t_max_err fl_batuta_update_uigoto(t_fl_batuta *x);
t_max_err fl_batuta_update_uibar(t_fl_batuta *x);
	//---perform
void fl_batuta_onoff(t_fl_batuta *x, long n);
void reset_cont_goto(t_fl_batuta *x);
void fl_batuta_nextbar(t_fl_batuta *x, long n);
void fl_batuta_rec(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv);
void fl_batuta_update_rec(t_fl_batuta *x);

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
//int getlinea(char *dest, char *orig, int lim);
	//---math
float parse_curve(float curva);
float msbeat_to_bpm(float ms_beat);
	//---comparison
long signature_prevbar(fl_tsign *a, fl_tsign *b);
long goto_prevbar(fl_goto *a, fl_goto *b);
long tempo_prevbar(fl_tempo *a, fl_tempo *b);
long note_prevstart(fl_note *a, fl_note *b);
long signature_samebar(fl_tsign *a, fl_tsign *b);
long goto_samebar(fl_goto *a, fl_goto *b);
long tempo_samebar(fl_tempo *a, fl_tempo *b);
