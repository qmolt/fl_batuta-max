#include "fl_batuta~.h"

void fl_batuta_new_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	long total_bars, index;
	t_atom *ap = argv;
	long ac = argc;

	if (x->isplaying) { object_error((t_object *)x, "bar: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "bar: can't edit while reading/writing a file"); return; }
	//(1 arg) add bar at position //(no args) add bar at the end
	if (ac > 1) { object_error((t_object *)x, "bar: \n[0 args] adds the bar at the end\n[1 arg] adds the bar at this index position"); return; }

	x->isediting = 1;

	total_bars = (long)linklist_getsize(x->l_bars);
	if (ac == 0) {		
		err = do_add_bar(x, total_bars);
	}
	else { 
		if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "bar: bar index must be a number (integer)"); x->isediting = 0; return; }
		index = (long)atom_getlong(ap);
		if (index < 0) { object_error((t_object *)x, "bar: index must be 0 or positive"); x->isediting = 0; return; }
		if (index > total_bars) {
			long dif = index - total_bars;
			for (int i = 0; i < dif; i++) {
				err = do_add_bar(x, total_bars + i);
				if (err) { object_error((t_object *)x, "bar: bar couldn't be added"); x->isediting = 0; return; }
			}
		}
		else {	
			err = do_add_bar(x, index);
		}
	}
	if (err) { object_error((t_object *)x, "bar: bar couldn't be added"); x->isediting = 0; return; }

	err = fl_batuta_update_notes(x);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }
	
	err = fl_batuta_update_uibar(x);
	err += fl_batuta_update_uitempo(x);
	err += fl_batuta_update_uigoto(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_add_bar(t_fl_batuta *x, long pos)
{
	//memory allocation
	fl_bar *pbar = (fl_bar *)sysmem_newptr(sizeof(fl_bar));
	if (!pbar) { return MAX_ERR_OUT_OF_MEM; }
	fl_note **hnota = (fl_note **)sysmem_newptr(sizeof(fl_note *));
	if (!hnota) { sysmem_freeptr(pbar); return MAX_ERR_OUT_OF_MEM; }
	//initialize bar
	pbar->total_notas = 0;
	pbar->hdl_nota = hnota;
	pbar->notas = linklist_new();
	linklist_flags(pbar->notas, OBJ_FLAG_MEMORY);
	pbar->isgoto_ui = -1;
	pbar->psignat_ui = NULL; 
	pbar->pgoto_ui = NULL; 
	pbar->ptempo_ui = NULL;
	//add bar to the list
	long index = (long)linklist_insertindex(x->l_bars, pbar, pos);
	if (index == -1) { sysmem_freeptr(pbar); sysmem_freeptr(hnota); return MAX_ERR_GENERIC; }
	return MAX_ERR_NONE;
}

void fl_batuta_delete_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long index;

	if (x->isplaying) { object_error((t_object *)x, "bar: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "bar: can't edit while reading/writing a file"); return; }
	if (ac != 1) { object_error((t_object *)x, "bar: [1 arg] bar index"); return; }
	//1 arg: bar index
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "bar: bar index must be a number"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	index = (long)atom_getlong(ap);

	if (index < 0 || index >= total_bars) { object_error((t_object *)x, "bar: bar index must be 0 or positive less than the total of bars"); return; }

	x->isediting = 1;

	err = do_delete_bar(x, index);
	if (err) { object_error((t_object *)x, "bar: bar couldn't be deleted"); x->isediting = 0; return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (!total_bars) { do_add_bar(x, 0); }

	err = fl_batuta_update_notes(x);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	err += fl_batuta_update_uitempo(x);
	err += fl_batuta_update_uigoto(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }
	
	x->isediting = 0;
}
t_max_err do_delete_bar(t_fl_batuta *x, long pos)
{
	t_max_err err = MAX_ERR_NONE;
	//get bar and notes at index
	fl_note *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, pos);
	if (!pbar) { return MAX_ERR_INVALID_PTR; }
	long n_notas = (long)linklist_getsize(pbar->notas);
	//delete notes
	for (int j = 0; j < n_notas; j++) {
		pnota = linklist_getindex(pbar->notas, j);
		if (!pnota) { return MAX_ERR_INVALID_PTR; }
		sysmem_freeptr(pnota->pnota); //free atom array
	}
	if (pbar->hdl_nota) {
		sysmem_freeptr(pbar->hdl_nota);
	}
	err = object_free(pbar->notas); //free memory for linked list in bar
	//delete bar from list
	long index = (long)linklist_deleteindex(x->l_bars, pos);
	if (index == -1) { return MAX_ERR_GENERIC; }
	return MAX_ERR_NONE;
}

void fl_batuta_copy_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long bar_orig;
	long bar_dest;

	if (x->isplaying) { object_error((t_object *)x, "bar: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "bar: can't edit while reading/writing a file"); return; }
	if (ac != 2) { object_error((t_object *)x, "bar: [2 args] index copied bar, index pasted bar"); return; }

	//2 arg: index_orig index_dest
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "bar: copied bar must be a number (integer)"); return; }
	if(atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) {object_error((t_object *)x, "bar: pasted bar must be a number (integer)"); return;}

	total_bars = (long)linklist_getsize(x->l_bars);
	bar_orig = (long)atom_getlong(ap);
	bar_dest = (long)atom_getlong(ap + 1);

	if (!total_bars) { object_error((t_object *)x, "bar: no bars created"); return; }
	if (bar_orig >= total_bars || bar_orig < 0) { object_error((t_object *)x, "bar: copied bar index must be 0 or a positive less than the total of bars"); return; }
	if (bar_dest >= total_bars || bar_dest < 0) { object_error((t_object *)x, "bar: pasted bar index must be 0 or a positive less than the total of bars"); return; }

	x->isediting = 1;

	err = do_copy_bar(x, bar_orig, bar_dest);
	if(err){ object_error((t_object *)x, "bar: bar couldn't be copied"); x->isediting = 0; return; }

	err = fl_batuta_update_notes(x);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	err += fl_batuta_update_uibar(x);
	err += fl_batuta_update_uigoto(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_copy_bar(t_fl_batuta *x, long orig, long dest)
{
	t_max_err err = MAX_ERR_NONE;
	long errindex = -1;
	long n_notas;
	fl_note *pnota_orig;
	fl_note *pnota_dest;
	fl_bar *pbar_orig = linklist_getindex(x->l_bars, orig);
	if (!pbar_orig) { return MAX_ERR_INVALID_PTR; }
	//initialize new bar
	fl_bar *pbar_dest = (fl_bar *)sysmem_newptr(sizeof(fl_bar));
	if (!pbar_dest) { return MAX_ERR_OUT_OF_MEM; }
	pbar_dest->notas = linklist_new();
	linklist_flags(pbar_dest->notas, OBJ_FLAG_MEMORY);
	pbar_dest->hdl_nota = (fl_note **)sysmem_newptr(sizeof(fl_note *));
	if (!pbar_dest->hdl_nota) { return MAX_ERR_OUT_OF_MEM; }
	//copy everything
	pbar_dest->total_notas = pbar_orig->total_notas;
	pbar_dest->psignat_ui = NULL;
	pbar_dest->ptempo_ui = NULL;
	pbar_dest->pgoto_ui = NULL;
	pbar_dest->isgoto_ui = -1;
	n_notas = (long)linklist_getsize(pbar_orig->notas);
	for (int j = 0; j < n_notas; j++) {
		pnota_orig = linklist_getindex(pbar_orig->notas, j);
		if (!pnota_orig) { return MAX_ERR_INVALID_PTR; }
		pnota_dest = (fl_note *)sysmem_newptr(sizeof(fl_note));
		if (!pnota_dest) { return MAX_ERR_OUT_OF_MEM; }
		pnota_dest->cnota = pnota_orig->cnota;
		pnota_dest->canal = pnota_orig->canal;
		pnota_dest->b_inicio = pnota_orig->b_inicio;
		pnota_dest->pnota = (t_atom *)sysmem_newptr((pnota_orig->cnota) * sizeof(t_atom));
		if (!pnota_dest->pnota) { return MAX_ERR_OUT_OF_MEM; }
		err = atom_getatom_array(pnota_orig->cnota, pnota_orig->pnota, pnota_dest->cnota, pnota_dest->pnota);
		if (err) { return MAX_ERR_GENERIC; }
		errindex = (long)linklist_insertindex(pbar_dest->notas, pnota_dest, j);
		if (errindex == -1) { sysmem_freeptr(pnota_dest->pnota); sysmem_freeptr(pnota_dest); return MAX_ERR_GENERIC; }
	}
	errindex = (long)linklist_insertindex(x->l_bars, pbar_dest, dest);
	if (errindex == -1) { return MAX_ERR_GENERIC; }
	return MAX_ERR_NONE;
}

void fl_batuta_move_bar(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long bar_dest;
	long bar_orig;
	long total_bars;

	if (x->isplaying) { object_error((t_object *)x, "bar: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "bar: can't edit while reading/writing a file"); return; }
	if (ac != 2) { object_error((t_object *)x, "bar: [2 args] cutted bar index, pasted bar index"); return; }
	//2 arg: index_orig index_dest
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT 
		&& atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, 
			"bar: cutted/pasted bars must be numbers"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	bar_orig = (long)atom_getlong(ap);
	bar_dest = (long)atom_getlong(ap + 1);

	if (!total_bars) { object_error((t_object *)x, "bar: no bars created"); return; }
	if (bar_orig >= total_bars || bar_orig < 0) { object_error((t_object *)x, 
		"bar: cutted bar index must be 0 or a positive less than the total of bars"); return; }
	if (bar_dest >= total_bars - 1 || bar_dest < 0) { object_error((t_object *)x, 
		"bar: pasted bar index must be 0 or a positive less than the total of bars - 1"); return; }
	if (bar_orig == total_bars) { object_error((t_object *)x, "bar: cutted bar index and pasted bar index can't be the same"); return; }

	x->isediting = 1;

	err = do_move_bar(x, bar_orig, bar_dest);
	if (err) { object_error((t_object *)x, "bar: bar couldn't be moved"); x->isediting = 0; return; }

	err = fl_batuta_update_notes(x);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }
	
	err = fl_batuta_update_uibar(x);
	err += fl_batuta_update_uitempo(x);
	err += fl_batuta_update_uigoto(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_move_bar(t_fl_batuta *x, long orig, long dest)
{
	fl_bar *pbar_orig = linklist_getindex(x->l_bars, orig);
	if (!pbar_orig) { return MAX_ERR_OUT_OF_MEM; }
	fl_bar *pbar_dest = linklist_getindex(x->l_bars, dest);
	if (!pbar_dest) { return MAX_ERR_OUT_OF_MEM; }
	if (orig > dest) {
		linklist_movebeforeobjptr(x->l_bars, pbar_orig, pbar_dest);
	}
	else if (orig < dest) {
		linklist_moveafterobjptr(x->l_bars, pbar_orig, pbar_dest);
	}
	return MAX_ERR_NONE;
}
//------------
void fl_batuta_delete_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_bar;
	long n_canal;

	if (x->isplaying) { object_error((t_object *)x, "chan: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "chan: can't edit while reading/writing a file"); return; }
	if (ac != 2) { object_error((t_object *)x, "chan: [2 args] bar index, channel number"); return; }
	//2 arg: compas y canal
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "chan: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "chan: channel number must be a number (integer)"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	n_bar = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);

	if (n_bar >= total_bars || n_bar < 0) { object_error((t_object *)x, "chan: bar index must be more than 0 and less than the total of bars"); return; }

	x->isediting = 1;

	err = do_delete_chan(x, n_bar, n_canal);
	if(err){ object_error((t_object *)x, "chan: chan couldn't be deleted"); x->isediting = 0; return; }

	err = fl_batuta_update_notes_onebar(x, n_bar);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_delete_chan(t_fl_batuta *x, long bar, long chan) {
	fl_note *pnota;
	long errindex = -1;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	if (!pbar) { return MAX_ERR_OUT_OF_MEM; }
	long total_notas = (long)linklist_getsize(pbar->notas);
	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (!pnota) { return MAX_ERR_INVALID_PTR; }
		if (pnota->canal == chan) {
			errindex = (long)linklist_deleteindex(pbar->notas, i);
			if (errindex == -1) { return MAX_ERR_GENERIC; }
			sysmem_freeptr(pnota->pnota);
		}
	}
	return MAX_ERR_NONE;
}

void fl_batuta_copy_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_bar_orig;
	long n_bar_dest;
	long n_canal;

	if (x->isplaying) { object_error((t_object *)x, "chan: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "chan: can't edit while reading/writing a file"); return; }
	if (ac != 3) { object_error((t_object *)x, "chan: [3 args] bar index, channel number, target bar"); return; }
	//3arg: n bar, chan, target bar
	total_bars = (long)linklist_getsize(x->l_bars);
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "chan: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "chan: channel must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "chan: bar index must be a number (integer)"); return; }

	n_bar_orig = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	n_bar_dest = (long)atom_getlong(ap + 2);

	if (n_bar_orig >= total_bars || n_bar_orig < 0) { object_error((t_object *)x, "chan: bar index must be more than 0 and less than the total of bars"); return; }
	if (n_bar_dest >= total_bars) { object_error((t_object *)x, "chan: target bar index must be less than the total of bars"); return; }

	x->isediting = 1;

	err = do_copy_chan(x, n_canal, n_bar_orig, n_bar_dest);
	if (err) { object_error((t_object *)x, "chan: chan couldn't be copied"); x->isediting = 0; return; }

	err = fl_batuta_update_notes_onebar(x, n_bar_dest);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_copy_chan(t_fl_batuta *x, long chan, long bar_orig, long bar_dest) {
	t_max_err err = MAX_ERR_NONE;
	long errindex = -1;
	fl_note *pnota_orig, *pnota_dest;
	fl_bar *pbar_orig = linklist_getindex(x->l_bars, bar_orig);
	if (!pbar_orig) { return MAX_ERR_INVALID_PTR; }
	fl_bar *pbar_dest = linklist_getindex(x->l_bars, bar_dest);
	if (!pbar_dest) { return MAX_ERR_INVALID_PTR; }
	long total_notas = (long)linklist_getsize(pbar_orig->notas);
	for (int j = 0; j < total_notas; j++) {
		pnota_orig = linklist_getindex(pbar_orig->notas, j);
		if (!pnota_orig) { return MAX_ERR_INVALID_PTR; }
		if (pnota_orig->canal == chan) {
			pnota_dest = (fl_note *)sysmem_newptr(sizeof(fl_note));
			if (!pnota_dest) { return MAX_ERR_OUT_OF_MEM; }
			pnota_dest->b_inicio = pnota_orig->b_inicio;
			pnota_dest->cnota = pnota_orig->cnota;
			pnota_dest->canal = pnota_orig->canal;
			pnota_dest->pnota = (t_atom *)sysmem_newptr((pnota_orig->cnota) * sizeof(t_atom));
			if (!pnota_dest->pnota) { return MAX_ERR_OUT_OF_MEM; }
			err = atom_getatom_array(pnota_orig->cnota, pnota_orig->pnota, pnota_dest->cnota, pnota_dest->pnota);
			if (err) { sysmem_freeptr(pnota_dest->pnota); sysmem_freeptr(pnota_dest); return MAX_ERR_GENERIC; }
			errindex = (long)linklist_insertindex(pbar_dest->notas, pnota_dest, 0);
			if (errindex == -1) { sysmem_freeptr(pnota_dest->pnota); sysmem_freeptr(pnota_dest); return MAX_ERR_GENERIC; }
		}
	}
	return MAX_ERR_NONE;
}

void fl_batuta_edit_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal_orig;
	long n_canal_dest;

	if (x->isplaying) { object_error((t_object *)x, "chan: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "chan: can't edit while reading/writing a file"); return; }
	//3arg: bar index, channel, new channel
	if (ac != 3) { object_error((t_object *)x, "chan: [3 args] bar index, channel number, new channel number"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (!total_bars) { object_error((t_object *)x, "chan: couldn't find any bar"); return; }

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "chan: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "chan: channel must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "chan: new channel must be anumber (integer)"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal_orig = (long)atom_getlong(ap + 1);
	n_canal_dest = (long)atom_getlong(ap + 2);

	if (n_compas >= total_bars || n_compas < 0) { object_error((t_object *)x, "chan: bar index must be more than 0 and less than the total of bars"); return; }

	x->isediting = 1;

	err = do_edit_chan(x, n_compas, n_canal_orig, n_canal_dest);
	if (err) { object_error((t_object *)x, "chan: channel couldn't be edited"); x->isediting = 0; return; }

	err = fl_batuta_update_notes_onebar(x, n_compas);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_edit_chan(t_fl_batuta *x, long bar, long chan_ini, long chan_fin)
{
	fl_note *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	if (!pbar) { return MAX_ERR_INVALID_PTR; }
	long total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { return MAX_ERR_GENERIC; }
	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (!pnota) { return MAX_ERR_INVALID_PTR; }
		if (pnota->canal == chan_ini) {
			if (!pnota->pnota) { return MAX_ERR_INVALID_PTR; }
			pnota->canal = chan_fin;
			atom_setlong(pnota->pnota, chan_fin);
		}
	}
	return MAX_ERR_NONE;
}
//------------
t_max_err do_find_note_index(t_fl_batuta *x, long *note_index, long bar, long chan, long note_in_chan) {
	fl_bar *pbar;
	fl_note *pnote;
	long total_notes;
	long total_bars;
	long index_accum;
	long n_bar = bar;
	long n_chan = chan;
	long n_note = note_in_chan; //index in channel
	long *index_note = note_index; //index in bar

	*index_note = -1;
	if (n_note < 0) { return MAX_ERR_GENERIC; }
	total_bars = (long)linklist_getsize(x->l_bars);
	if (n_bar < 0 || n_bar >= total_bars) { return MAX_ERR_GENERIC; }
	pbar = linklist_getindex(x->l_bars, n_bar);
	total_notes = (long)linklist_getsize(pbar->notas);

	index_accum = 0;
	for (int i = 0; i < total_notes; i++) {
		pnote = linklist_getindex(pbar->notas, i);
		if (pnote->canal == n_chan) {
			if (index_accum++ == n_note) {
				*index_note = i;
			}
		}
	}
	return MAX_ERR_NONE;
}
void fl_batuta_new_note(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal;
	float b_inicio;
	t_max_err err;

	if (x->isplaying) { object_error((t_object *)x, "note: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "note: can't edit while reading/writing a file"); return; }
	//min 4 arg: bar index, start, channel, info... 
	if (ac < 4) { object_error((t_object *)x, "note: [4 args min] bar index, channel number, start beat, info..."); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (total_bars == 0) { object_error((t_object *)x, "note: couldn't find any bar"); return; }

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "note: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "note: channel number must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "note: start beat must be a number (decimal)"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	b_inicio = (float)atom_getfloat(ap + 2);

	if (n_compas >= total_bars || n_compas < 0) { object_error((t_object *)x, "note: bar index must be more than 0 and less than the total of bars"); return; }
	if (b_inicio < 0.0) { object_error((t_object *)x, "note: start must be 0 or positive"); return; }
	
	x->isediting = 1;

	atom_setlong(ap + 2, n_canal);

	err = do_add_note(x, n_compas, b_inicio, n_canal, ac - 3, ap + 3);
	if (err) { object_error((t_object *)x, "note: note couldn't be added"); x->isediting = 0; return; }
	
	err = fl_batuta_update_notes_onebar(x, n_compas);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_add_note(t_fl_batuta *x, long bar, float inicio, long canal, long listac, t_atom *listav) {
	t_max_err err = MAX_ERR_NONE;
	long index = -1;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	fl_note *pnota = (fl_note *)sysmem_newptr(sizeof(fl_note));
	if (!pnota) { return MAX_ERR_OUT_OF_MEM; }
	pnota->b_inicio = inicio;
	pnota->canal = canal;
	pnota->pnota = (t_atom *)sysmem_newptr((listac + 1) * sizeof(t_atom));
	if (!pnota->pnota) { sysmem_freeptr(pnota); return MAX_ERR_OUT_OF_MEM; }
	pnota->cnota = (listac + 1);
	atom_setlong(pnota->pnota, canal);
	err = atom_getatom_array(listac, listav, pnota->cnota - 1, pnota->pnota + 1);
	if (err) { sysmem_freeptr(pnota->pnota); sysmem_freeptr(pnota); return MAX_ERR_GENERIC; }
	index = (long)linklist_insertindex(pbar->notas, pnota, 0);
	if (index == -1) { return MAX_ERR_GENERIC; }
	return MAX_ERR_NONE;
}

void fl_batuta_delete_note(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal;
	long index_borrar;
	long index;
	long total_notas;
	fl_bar *pbar;
	t_max_err err;

	if (x->isplaying) { object_error((t_object *)x, "note: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "note: can't edit while reading/writing a file"); return; }
	//3 arg: bar index, channel, note position
	if (ac != 3) { object_error((t_object *)x, "note: [3 args] bar index, channel number, note index (zero-wise)"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "note: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "note: channel number must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "note: note index must be a number (integer)"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	index_borrar = (long)atom_getlong(ap + 2);
	total_bars = (long)linklist_getsize(x->l_bars);

	if (n_compas >= total_bars || n_compas < 0) { object_error((t_object *)x, "note: bar index must be 0 or positive less than the total of bars"); return; }

	pbar = linklist_getindex(x->l_bars, n_compas);
	total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "note: couldn't find any note in this bar"); return; }
	if (index_borrar >= total_notas || index_borrar < 0) { object_error((t_object *)x, "note: bar index must be 0 or positive less than the total of notes"); return; }

	index = -1;
	do_find_note_index(x, &index, n_compas, n_canal, index_borrar); //note index in chan, to note index in bar
	if (index == -1) { object_error((t_object *)x, "note: couldn't find note index in bar"); return; }

	x->isediting = 1;

	err = do_delete_note(x, n_compas, index_borrar);
	if (err) { object_error((t_object *)x, "note: couldn't delete note"); x->isediting = 0; return; }

	err = fl_batuta_update_notes_onebar(x, n_compas);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_delete_note(t_fl_batuta *x, long bar, long index_nota) {
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	if (!pbar) { return MAX_ERR_INVALID_PTR; }
	long total_notas = (long)linklist_getsize(pbar->notas);
	long errindex = (long)linklist_deleteindex(pbar->notas, index_nota);
	if (errindex == -1) { return MAX_ERR_GENERIC; }
	return MAX_ERR_NONE;
}

void fl_batuta_edit_note(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal;
	long index_editar;
	long index;
	long total_notas;
	fl_bar *pbar;
	t_max_err err;

	if (x->isplaying) { object_error((t_object *)x, "note: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "note: can't edit while reading/writing a file"); return; }
	//min 3 arg: bar index, channel, note position, info...
	if (ac < 4) { object_error((t_object *)x, "note: [4 args min] bar index, channel number, note index, info..."); return; }

	total_bars = (long)linklist_getsize(x->l_bars);

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "note: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "note: channel number must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "note: note index must be a number (integer)"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	index_editar = (long)atom_getlong(ap + 2);

	if (n_compas < 0 || n_compas >= total_bars) { object_error((t_object *)x, "note: bar index must be 0 or positive less than the total of bars"); return; }

	pbar = linklist_getindex(x->l_bars, n_compas);
	total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "note: couldn't find any notes in this channel"); return; }
	if (index_editar < 0 || index_editar > total_notas) { object_error((t_object *)x, "note: note index must be 0 or positive less than the total of notes"); return; }

	index = -1;
	do_find_note_index(x, &index, n_compas, n_canal, index_editar);
	if (index == -1) { object_error((t_object *)x, "note: couldn't find note index in bar"); return; }

	x->isediting = 1;

	err = do_edit_note_info(x, n_compas, index, ac - 3, ap + 3);
	if (err) { object_error((t_object *)x, "note: couldn't edit note"); x->isediting = 0; return; }

	err = fl_batuta_update_notes_onebar(x, n_compas);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }
	
	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_edit_note_info(t_fl_batuta *x, long bar, long index_nota, long listac, t_atom *listav) {
	fl_note *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	t_max_err err = MAX_ERR_NONE;
	long total_notas = (long)linklist_getsize(pbar->notas);
	pnota = linklist_getindex(pbar->notas, index_nota);
	if (!pnota) { return MAX_ERR_INVALID_PTR; }
	if (!pnota->pnota) { pnota->pnota = (t_atom *)sysmem_newptr((listac + 1) * sizeof(t_atom)); }
	else { pnota->pnota = (t_atom *)sysmem_resizeptr(pnota->pnota, (listac + 1) * sizeof(t_atom)); }
	if (!pnota->pnota) { return MAX_ERR_OUT_OF_MEM; }
	pnota->cnota = listac + 1;
	err = atom_getatom_array(listac, listav, pnota->cnota - 1, pnota->pnota + 1);
	if (err) { return MAX_ERR_GENERIC; }
	return MAX_ERR_NONE;
}
t_max_err do_edit_note_start(t_fl_batuta *x, long bar, long index_nota, float inicio) {
	fl_note *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	if (!pbar) { return MAX_ERR_INVALID_PTR; }
	long total_notas = (long)linklist_getsize(pbar->notas);
	pnota = linklist_getindex(pbar->notas, index_nota);
	if (!pnota) { return MAX_ERR_INVALID_PTR; }
	pnota->b_inicio = inicio;
	return MAX_ERR_NONE;
}
t_max_err do_edit_note_chan(t_fl_batuta *x, long bar, long index_nota, long canal) {
	fl_note *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	if (!pbar) { return MAX_ERR_INVALID_PTR; }
	long total_notas = (long)linklist_getsize(pbar->notas);
	pnota = linklist_getindex(pbar->notas, index_nota);
	if (!pnota) { return MAX_ERR_INVALID_PTR; }
	pnota->canal = canal;
	atom_setlong(pnota->pnota, canal);
	return MAX_ERR_NONE;
}
//------------
void fl_batuta_new_signature(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	float beats;
	long n_bar;
	long total_tsign;
	fl_tsign *ptsign;

	if (x->isplaying) { object_error((t_object *)x, "signature: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "signature: can't edit while reading/writing a file"); return; }
	//2 arg: n compas; beats
	if (ac != 2) { object_error((t_object *)x, "signature: [2 args] n bar, beats"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "signature: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "signature: beats must be a number (decimal)"); return; }

	n_bar = (long)atom_getlong(ap);
	beats = (float)atom_getfloat(ap + 1);

	if (n_bar < 0) { object_error((t_object *)x, "signature: bar index must be 0 or a positive"); return; }
	if (beats < 1.) { object_error((t_object *)x, "signature: beats must be positive greater or equal than 1."); return; }

	x->isediting = 1;

	total_tsign = (long)linklist_getsize(x->l_tsigns);
	if (!total_tsign) { 
		err = do_add_signature(x, 0, 4.); 
		if (err) { object_error((t_object *)x, "signature: add a tsign on the first bar"); }
	}
	ptsign = linklist_getindex(x->l_tsigns, 0);
	if (ptsign) {
		if (ptsign->n_bar != 0) {
			err = do_add_signature(x, 0, 4.);
			if (err) { object_error((t_object *)x, "signature: add a signature on the first bar"); }
		}
	}

	err = do_add_signature(x, n_bar, beats);
	if (err) { object_error((t_object *)x, "signature: signature couldn't be added"); x->isediting = 0; return; }

	err = fl_batuta_update_signatures(x);
	if (err) { object_error((t_object *)x, "signature: signatures couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "signature: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
 t_max_err do_add_signature(t_fl_batuta *x, long bar, float beats) {
	long index;
	float n_beats = (float)MAX(beats, 1.);
	long total_tsigns = (long)linklist_getsize(x->l_tsigns);
	fl_tsign *foundcifra = NULL;
	//probe signature
	fl_tsign *pcifra = (fl_tsign *)sysmem_newptr(sizeof(fl_tsign));
	if (!pcifra) { return MAX_ERR_OUT_OF_MEM; }
	pcifra->n_bar = bar;
	//find if there's already a signature in that bar
	if (total_tsigns) { index = (long)linklist_findfirst(x->l_tsigns, &foundcifra, signature_samebar, pcifra); }
	else { index = -1; }
	//create a new signature if not found or if first in list; replace if a signature is found in that bar
	if (index != -1) {
		foundcifra->beats = n_beats;
		sysmem_freeptr(pcifra);
	}
	else {
		pcifra->beats = n_beats;
		long errindex = (long)linklist_insertindex(x->l_tsigns, pcifra, 0);
		if (errindex == -1) { sysmem_freeptr(pcifra); return MAX_ERR_GENERIC; }
		linklist_sort(x->l_tsigns, signature_prevbar);
	}
	return MAX_ERR_NONE;
}

void fl_batuta_delete_signature(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long n_compas;
	long total_tsign;
	fl_tsign *ptsign;

	if (x->isplaying) { object_error((t_object *)x, "signature: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "signature: can't edit while reading/writing a file"); return; }
	if (ac != 1) { object_error((t_object *)x, "signature: [1 arg] index bar"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "signature: index bar must be a number"); return; }

	n_compas = (long)atom_getlong(ap);

	if (n_compas < 0) { object_error((t_object *)x, "signature: index bar must be 0 or positive"); return; }

	x->isediting = 1;

	err = do_delete_signature(x, n_compas);
	if (err) { object_error((t_object *)x, "signature: signature couldn't be deleted"); x->isediting = 0; return; }

	total_tsign = (long)linklist_getsize(x->l_tsigns);
	if (!total_tsign) { 
		err = do_add_signature(x, 0, 4.); 
		if (err) { object_error((t_object *)x, "signature: add a tsign on the first bar"); }
	}
	ptsign = linklist_getindex(x->l_tsigns, 0);
	if (ptsign) {
		if (ptsign->n_bar != 0) {
			err = do_add_signature(x, 0, 4.);
			if (err) { object_error((t_object *)x, "signature: add a tsign on the first bar"); }
		}
	}

	err = fl_batuta_update_signatures(x);
	if (err) { object_error((t_object *)x, "signature: signatures couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "signature: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_delete_signature(t_fl_batuta *x, long bar) {
	long index, total_tsigns;
	fl_tsign *pcifra, *foundcifra;
	total_tsigns = (long)linklist_getsize(x->l_tsigns);
	if (total_tsigns) {
		pcifra = (fl_tsign *)sysmem_newptr(sizeof(fl_tsign));
		if (!pcifra) { return MAX_ERR_OUT_OF_MEM; }
		pcifra->n_bar = bar;
		index = (long)linklist_findfirst(x->l_tsigns, &foundcifra, signature_samebar, pcifra);
		sysmem_freeptr(pcifra);
		if (index == -1) { return MAX_ERR_GENERIC; }
		long errindex = (long)linklist_deleteindex(x->l_tsigns, index);
		if (errindex == -1) { return MAX_ERR_GENERIC; }
	}
	return MAX_ERR_NONE;
}

//------------
void fl_batuta_new_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//[2 arg] n bar; ms beat
	//[3 arg] n bar; ms beat; ms start						//(type 0)instant change
	//[4 arg] n bar; ms beat; ms start; durat dtempo		//(type 1)line format change
	//[5 arg] n bar; ms beat; ms start; durat dtempo; curve	//(type 2)curve format change
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	short type;
	long n_bar;
	float ms_inicio, ms_beat, ms_durvar, curva;
	long total_tempos;
	fl_tempo *ptempo;

	if (x->isplaying) { object_error((t_object *)x, "tempo: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "tempo: can't edit while reading/writing a file"); return; }
	if (ac < 2 || ac > 5) { object_error((t_object *)x, "tempo: \n"
		"[2 args] index bar, ms beat\n"
		"[3 args] index bar, ms beat, ms delay\n"
		"[4 args] index bar, ms beat, ms delay, duration\n"
		"[5 args] index bar, ms beat, ms delay, duration, curve"); return; }

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "tempo: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_FLOAT && atom_gettype(ap + 1) != A_LONG) { object_error((t_object *)x, "tempo: ms beat (period) must be a number (decimal)"); return; }
	
	type = 0;
	n_bar = (long)atom_getlong(ap);
	ms_beat = (float)atom_getfloat(ap + 1);
	ms_inicio = 0.f;
	ms_durvar = 0.f;
	curva = 0.f;
	
	if (ac > 2) {
		type = 0;
		if (atom_gettype(ap + 2) != A_FLOAT && atom_gettype(ap + 2) != A_LONG) { object_error((t_object *)x, "tempo: ms delay to start of change must be a number (decimal)"); return; }
		ms_inicio = (float)atom_getfloat(ap + 2);
	}
	if (ac > 3) {
		type = 1;
		if (atom_gettype(ap + 3) != A_FLOAT && atom_gettype(ap + 3) != A_LONG) { object_error((t_object *)x, "tempo: ms duracion cambio debe ser float"); return; }
		ms_durvar = (float)atom_getfloat(ap + 3);
	}
	if (ac > 4) {
		type = 2;
		if (atom_gettype(ap + 4) != A_FLOAT && atom_gettype(ap + 4) != A_LONG) { object_error((t_object *)x, "tempo: curva debe ser float"); return; }
		curva = (float)atom_getfloat(ap + 4);
		curva = MIN(MAX(curva, CURVE_MIN), CURVE_MAX);
	}
	
	if (n_bar < 0) { object_error((t_object *)x, "tempo: bar index must be 0 or a positive"); return; }
	if (ms_inicio < 0.) { object_error((t_object *)x, "tempo: start delay must be 0 or a positive"); return; }
	if (ms_beat < TEMPO_MIN) { object_error((t_object *)x, "tempo: beat period must be a positive more than 1. millisecond"); return; }

	x->isediting = 1;

	if (n_bar == 0 && ac > 2) { ms_inicio = 0.f; object_warn((t_object *)x, "tempo: delay set to 0.ms on first bar"); }
	
	total_tempos = (long)linklist_getsize(x->l_tempos);
	if (!total_tempos) { 
		err = do_add_tempo(x, 0, 0, 0., 500., 0., 0.); 
		if (err) { object_error((t_object *)x, "tempo: add a tempo on the first bar"); }
	}
	ptempo = linklist_getindex(x->l_tempos, 0);
	if (ptempo) {
		if (ptempo->n_bar != 0) {
			err = do_add_tempo(x, 0, 0, 0., 500., 0., 0.);
			if (err) { object_error((t_object *)x, "tempo: add a tempo on the first bar"); }
		}
	}

	err = do_add_tempo(x, type, n_bar, ms_inicio, ms_beat, ms_durvar, curva);
	if (err) { object_error((t_object *)x, "tempo: tempo couldn't be added"); x->isediting = 0; return; }

	err = fl_batuta_update_tempos(x);
	if (err) { object_error((t_object *)x, "tempo: tempos couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uitempo(x);
	if (err) { object_error((t_object *)x, "tempo: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_add_tempo(t_fl_batuta *x, short tipo, long bar, float inicio, float msbeat, float var, float cur) {
	long total_tempos, index;
	fl_tempo *foundtempo = NULL;
	fl_tempo *ptempo;
	float ms_beat = MAX(msbeat, TEMPO_MIN);

	total_tempos = (long)linklist_getsize(x->l_tempos);
	//probe tempo
	ptempo = (fl_tempo *)sysmem_newptr(sizeof(fl_tempo));
	if (!ptempo) { return MAX_ERR_OUT_OF_MEM; }
	ptempo->n_bar = bar;
	//search if there's already a tempo on the bar index
	if (total_tempos) { index = (long)linklist_findfirst(x->l_tempos, &foundtempo, tempo_samebar, ptempo); }
	else { index = -1; }
	//if found change the info, if not found add a tempo
	if (index != -1) {
		sysmem_freeptr(ptempo);
		foundtempo->type = tipo;
		foundtempo->ms_inicio = inicio;
		foundtempo->ms_beat = ms_beat;
		foundtempo->ms_durvar = var;
		foundtempo->curva = cur;
	}
	else {
		ptempo->type = tipo;
		ptempo->ms_inicio = inicio;
		ptempo->ms_beat = ms_beat;
		ptempo->ms_durvar = var;
		ptempo->curva = cur;
		long errindex = (long)linklist_insertindex(x->l_tempos, ptempo, 0);
		if (errindex == -1) { sysmem_freeptr(ptempo); return MAX_ERR_GENERIC; }
		linklist_sort(x->l_tempos, tempo_prevbar);
	}
	return MAX_ERR_NONE;
}

void fl_batuta_delete_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//1 arg: n compas;
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long n_compas;
	long total_tempos;
	fl_tempo *ptempo;

	if (x->isplaying) { object_error((t_object *)x, "tempo: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "tempo: can't edit while reading/writing a file"); return; }
	if (ac != 1) { object_error((t_object *)x, "tempo: [1 arg] bar index"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "tempo: bar index must be a number (integer)"); return; }

	n_compas = (long)atom_getlong(ap);

	if (n_compas < 0) { object_error((t_object *)x, "tempo: bar index must be 0 or positive"); return; }

	x->isediting = 1;

	err = do_delete_tempo(x, n_compas);
	if (err) { object_error((t_object *)x, "tempo: tempo couldn't be deleted"); x->isediting = 0; return; }

	total_tempos = (long)linklist_getsize(x->l_tempos);
	if (!total_tempos) {
		err = do_add_tempo(x, 0, 0, 0., 500., 0., 0.);
		if (err) { object_error((t_object *)x, "tempo: add a tempo on the first bar"); }
	}
	ptempo = linklist_getindex(x->l_tempos, 0);
	if (ptempo) {
		if (ptempo->n_bar != 0) {
			err = do_add_tempo(x, 0, 0, 0., 500., 0., 0.);
			if (err) { object_error((t_object *)x, "tempo: add a tempo on the first bar"); }
		}
	}

	err = fl_batuta_update_tempos(x);
	if (err) { object_error((t_object *)x, "tempo: tempos couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uitempo(x);
	if (err) { object_error((t_object *)x, "tempo: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_delete_tempo(t_fl_batuta *x, long bar) {
	long index, total_tempos;
	fl_tempo *foundtempo, *ptempo;

	total_tempos = (long)linklist_getsize(x->l_tempos);
	if (total_tempos) {
		ptempo = (fl_tempo *)sysmem_newptr(sizeof(fl_tempo));
		if (!ptempo) { return MAX_ERR_OUT_OF_MEM; }
		ptempo->n_bar = bar;
		index = (long)linklist_findfirst(x->l_tempos, &foundtempo, tempo_samebar, ptempo);
		sysmem_freeptr(ptempo);
		if (index == -1) { return MAX_ERR_GENERIC; }

		long errindex = (long)linklist_deleteindex(x->l_tempos, index);
		if (errindex == -1) { return MAX_ERR_GENERIC; }
	}
	return MAX_ERR_NONE;
}
//------------

void fl_batuta_new_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long n_bar, total_rep, to_bar;

	if (x->isplaying) { object_error((t_object *)x, "goto: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "goto: can't edit while reading/writing a file"); return; }
	//3 arg: n bar; n to bar; n repet
	if (ac != 3) { object_error((t_object *)x, "goto: [3 args] bar index, target bar, n repetitions"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "goto: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "goto: target bar must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "goto: repetitions must be a number (integer)"); return; }

	n_bar = (long)atom_getlong(ap);
	to_bar = (long)atom_getlong(ap + 1);
	total_rep = (long)atom_getlong(ap + 2);

	if (n_bar < 0 || to_bar < 0) { object_error((t_object *)x, "goto: bar index must be 0 or positive"); return; }
	if (total_rep < 0) { object_error((t_object *)x, "goto: repetitions must be positive"); return; }

	x->isediting = 1;

	err = do_add_goto(x, n_bar, to_bar, total_rep);
	if (err) { object_error((t_object *)x, "goto: goto couldn't be added"); x->isediting = 0; return; }

	err = fl_batuta_update_gotos(x);
	if (err) { object_error((t_object *)x, "goto: gotos couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uigoto(x);
	if (err) { object_error((t_object *)x, "tempo: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_add_goto(t_fl_batuta *x, long bar_orig, long bar_dest, long repet) {
	fl_goto *pgoto;
	fl_goto *foundgoto = NULL;
	long index, total_gotos;

	total_gotos = (long)linklist_getsize(x->l_gotos);
	//probe goto
	pgoto = (fl_goto *)sysmem_newptr(sizeof(fl_goto));
	if (!pgoto) { return MAX_ERR_OUT_OF_MEM; }
	pgoto->n_bar = bar_orig;
	//find if goto in bar index
	if (total_gotos) { index = (long)linklist_findfirst(x->l_gotos, &foundgoto, goto_samebar, pgoto); }
	else { index = -1; }
	//if goto found replace values, if not found create a new one
	if (index != -1) {
		sysmem_freeptr(pgoto);
		foundgoto->cont_rep = 0;
		foundgoto->n_bar = bar_orig;
		foundgoto->to_bar = bar_dest;
		foundgoto->total_rep = repet;
	}
	else {
		pgoto->cont_rep = 0;
		pgoto->n_bar = bar_orig;
		pgoto->to_bar = bar_dest;
		pgoto->total_rep = repet;
		long errindex = (long)linklist_insertindex(x->l_gotos, pgoto, 0);
		if (errindex == -1) { return MAX_ERR_GENERIC; }
		linklist_sort(x->l_gotos, goto_prevbar);
	}
	return MAX_ERR_NONE;
}

void fl_batuta_delete_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long n_compas;

	if (x->isplaying) { object_error((t_object *)x, "goto: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "goto: can't edit while reading/writing a file"); return; }
	//1 arg: bar index
	if (ac != 1) { object_error((t_object *)x, "goto: [1 arg] bar index"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "goto: n compas debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);

	if (n_compas < 0) { object_error((t_object *)x, "goto: bar index must be 0 or positive"); return; }

	x->isediting = 1;

	err = do_delete_goto(x, n_compas);
	if (err) { object_error((t_object *)x, "goto: goto couldn't be added"); x->isediting = 0; return; }

	err = fl_batuta_update_gotos(x);
	if (err) { object_error((t_object *)x, "goto: gotos couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uigoto(x);
	if (err) { object_error((t_object *)x, "tempo: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_delete_goto(t_fl_batuta *x, long bar) {
	fl_goto *pgoto, *foundgoto;
	long total_gotos, index;

	total_gotos = (long)linklist_getsize(x->l_gotos);
	if (total_gotos) {
		pgoto = (fl_goto *)sysmem_newptr(sizeof(fl_goto));
		if (!pgoto) { return MAX_ERR_OUT_OF_MEM; }
		pgoto->n_bar = bar;
		index = (long)linklist_findfirst(x->l_gotos, &foundgoto, goto_samebar, pgoto);
		sysmem_freeptr(pgoto);
		if (index == -1) { return MAX_ERR_GENERIC; }

		long errindex = (long)linklist_deleteindex(x->l_gotos, index);
		if (errindex) { return MAX_ERR_GENERIC; }
	}
	return MAX_ERR_NONE;
}

//--------------------
void fl_batuta_quantize_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	t_atom *ap = argv;
	long ac = argc;
	long n_compas, n_canal;
	float div;
	long total_bars;

	if (x->isplaying) { object_error((t_object *)x, "quant: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "quant: can't edit while reading/writing a file"); return; }
	//3 args: bar index, chan, division of quarter note
	if (ac != 3 || !ap) { object_error((t_object *)x, "quant: [3 args] bar index, channel, divisions of a quarter"); return; }
	if (atom_gettype(ap) != A_FLOAT && atom_gettype(ap) != A_LONG) { object_error((t_object *)x, "quant: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_FLOAT && atom_gettype(ap + 1) != A_LONG) { object_error((t_object *)x, "quant: channel must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_FLOAT && atom_gettype(ap + 2) != A_LONG) { object_error((t_object *)x, "quant: division of quarter must be a number (decimal)"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (!total_bars) { object_error((t_object *)x, "quant: no bars created"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	div = (float)atom_getfloat(ap + 2);

	if (n_compas < 0 || n_compas >= total_bars) { object_error((t_object *)x, "quant: bar index must be 0 or a positive less than the total of bars"); return; }
	if (div < 1) { object_error((t_object *)x, "quant: division must be a positive equal or greater than 1"); return; }

	x->isediting = 1;

	err = do_quantize_chan(x, n_compas, n_canal, div);
	if (err) { object_error((t_object *)x, "quant: the channel couldn't be quantized"); x->isediting = 0; return; }

	err = fl_batuta_update_notes_onebar(x, n_compas);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_quantize_chan(t_fl_batuta *x, long bar, long chan, float div)
{
	long total_notas, total_tsigns;
	fl_bar *pbar;
	fl_note *pnota;
	fl_tsign *pcifra;
	long index_cifrafound, mayor_cifrafound;
	float cifra_found;
	//find bar
	pbar = linklist_getindex(x->l_bars, bar);
	if (!pbar) { return MAX_ERR_INVALID_PTR; }
	//find signature
	mayor_cifrafound = -1;
	index_cifrafound = -1;
	total_tsigns = (long)linklist_getsize(x->l_tsigns);
	for (int i = 0; i < total_tsigns; i++) {
		pcifra = linklist_getindex(x->l_tsigns, i);
		if (!pcifra) { return MAX_ERR_INVALID_PTR; }
		if (pcifra->n_bar >= mayor_cifrafound && pcifra->n_bar <= bar) {
			mayor_cifrafound = pcifra->n_bar;
			index_cifrafound = i;
		}
	}
	if (index_cifrafound == -1) { return MAX_ERR_GENERIC; }
	pcifra = linklist_getindex(x->l_tsigns, index_cifrafound);
	if (!pcifra) { return MAX_ERR_INVALID_PTR; }
	cifra_found = pcifra->beats;
	//quantize
	total_notas = (long)linklist_getsize(pbar->notas);
	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (!pnota) { return MAX_ERR_INVALID_PTR; }
		if (pnota->canal == chan) {
			pnota->b_inicio = (float)round((double)pnota->b_inicio * (double)div) / (float)div;
		}
	}
	return MAX_ERR_NONE;
}

void fl_batuta_human_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_max_err err = MAX_ERR_NONE;
	float margen_fijo, margen_random_max;
	t_atom *ap = argv;
	long ac = argc;
	long n_compas, n_canal, total_bars;

	if (x->isplaying) { object_error((t_object *)x, "human: can't edit while playing"); return; }
	if (x->isloading) { object_error((t_object *)x, "human: can't edit while reading/writing a file"); return; }
	if (argc != 4) { object_error((t_object *)x, "human: [4 args] bar index, channel, (+/-)fixed range(%%beat), (+/-)max random range(%%beat)"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "human: bar index must be a number (integer)"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "human: channel must be a number (integer)"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "human: fixed range must be a number (decimal%%)"); return; }
	if (atom_gettype(ap + 3) != A_LONG && atom_gettype(ap + 3) != A_FLOAT) { object_error((t_object *)x, "human: max random range must be a number (decimal%%)"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (!total_bars) { object_error((t_object *)x, "human: no bars created"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	margen_fijo = (float)(atom_getfloat(ap + 2) / 100.0);
	margen_random_max = (float)(atom_getfloat(ap + 3) / 100.0);

	if (n_compas < 0 || n_compas >= total_bars) { object_error((t_object *)x, "human: bar index must be 0 or a positive less than the total of bars"); return; }

	x->isediting = 1;

	err = do_human_chan(x, n_compas, n_canal, margen_fijo, margen_random_max);
	if (err) { object_error((t_object *)x, "human: channel couldn't be humanized"); x->isediting = 0; return; }

	err = fl_batuta_update_notes(x);
	if (err) { object_error((t_object *)x, "notes: notes couldn't be sorted."); x->isediting = 0; return; }

	err = fl_batuta_update_uibar(x);
	if (err) { object_error((t_object *)x, "notes: UI couldn't be updated."); x->isediting = 0; return; }

	x->isediting = 0;
}
t_max_err do_human_chan(t_fl_batuta *x, long bar, long chan, float d_fijo, float d_random)
{
	t_max_err err = MAX_ERR_NONE;
	long total_bars, total_notas;
	fl_bar *pbar;
	fl_note *pnota;
	long compasmayor_cifrafound, indexcompas_cifrafound, total_tsigns;
	float cifra_compas_ant, cifra_compas;
	fl_tsign *pcifra;
	float margen_random, b_inicio;
	long errindex;

	total_bars = (long)linklist_getsize(x->l_bars);
	pbar = linklist_getindex(x->l_bars, bar);
	if (!pbar) { return MAX_ERR_INVALID_PTR; }

	//find signature
	compasmayor_cifrafound = -1;
	indexcompas_cifrafound = -1;
	total_tsigns = (long)linklist_getsize(x->l_tsigns);
	for (int i = 0; i < total_tsigns; i++) {
		pcifra = linklist_getindex(x->l_tsigns, i);
		if (!pcifra) { return MAX_ERR_INVALID_PTR; }
		if (pcifra->n_bar >= compasmayor_cifrafound && pcifra->n_bar <= bar) {
			compasmayor_cifrafound = pcifra->n_bar;
			indexcompas_cifrafound = i;
		}
	}
	if (indexcompas_cifrafound == -1) { return MAX_ERR_GENERIC; }
	pcifra = linklist_getindex(x->l_tsigns, indexcompas_cifrafound);
	if (!pcifra) { return MAX_ERR_INVALID_PTR; }
	cifra_compas = pcifra->beats;

	//signature previous bar
	if (bar) {
		compasmayor_cifrafound = -1;
		indexcompas_cifrafound = -1;
		total_tsigns = (long)linklist_getsize(x->l_tsigns);
		for (int i = 0; i < total_tsigns; i++) {
			pcifra = linklist_getindex(x->l_tsigns, i);
			if (!pcifra) { return MAX_ERR_INVALID_PTR; }
			if (pcifra->n_bar >= compasmayor_cifrafound && pcifra->n_bar <= bar) {
				compasmayor_cifrafound = pcifra->n_bar;
				indexcompas_cifrafound = i;
			}
		}
		if (indexcompas_cifrafound == -1) { return MAX_ERR_GENERIC; }
		pcifra = linklist_getindex(x->l_tsigns, indexcompas_cifrafound);
		if (!pcifra) { return MAX_ERR_INVALID_PTR; }
		cifra_compas_ant = pcifra->beats;
	}

	//humanize
	total_notas = (long)linklist_getsize(pbar->notas);
	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (!pnota) { return MAX_ERR_INVALID_PTR; }
		if (chan != pnota->canal) { continue; }

		margen_random = (float)(d_random * ((rand() % 100) / 100.0));
		b_inicio = pnota->b_inicio;
		b_inicio = b_inicio + d_fijo + margen_random;

		if (b_inicio < 0) {
			if (!bar) {
				pnota->b_inicio = 0.0;
			}
			else {
				b_inicio += cifra_compas_ant;
				pnota->b_inicio = b_inicio;

				err = linklist_chuckindex(pbar->notas, i);
				if (err) { return MAX_ERR_GENERIC; }
				pbar = linklist_getindex(x->l_bars, bar - 1);
				if (!pbar) { return MAX_ERR_INVALID_PTR; }
				long errindex = (long)linklist_insertindex(pbar->notas, pnota, 0);
				if (errindex == -1) { return MAX_ERR_GENERIC; }
			}
		}
		else if (b_inicio >= cifra_compas) {
			b_inicio -= cifra_compas;
			pnota->b_inicio = b_inicio;

			err = linklist_chuckindex(pbar->notas, i);
			if (err) { return MAX_ERR_GENERIC; }

			if (bar == total_bars - 1) { 
				err = do_add_bar(x, total_bars); 
				if (err) { return MAX_ERR_GENERIC; }
			}
			
			pbar = linklist_getindex(x->l_bars, bar + 1);
			if (!pbar) { return MAX_ERR_INVALID_PTR; }
			errindex = (long)linklist_insertindex(pbar->notas, pnota, 0);
			if (errindex) { return MAX_ERR_GENERIC; }
		}
		else {
			pnota->b_inicio = b_inicio;
		}
	}
	return MAX_ERR_NONE;
}
