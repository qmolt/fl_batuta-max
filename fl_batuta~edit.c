#include "fl_batuta~.h"

void fl_batuta_nuevo_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//sin arg: al final
	//1 argum: posicion compas
	long total_bars, index;
	t_atom *ap = argv;
	long ac = argc;

	if (x->onoff) { object_error((t_object *)x, "no se puede editar en modo play"); return; }
	if (ac > 1) { object_error((t_object *)x, "nuevo compas recibe 0 a 1 elementos: 0 agrega compas al final, 1 agrega compas en dicho indice"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (ac == 0) {
		do_add_bar(x, total_bars);
	}
	else if (ac == 1) {
		if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "compas: n compas debe ser entero"); return; }
		index = (long)atom_getlong(ap);
		if (index >= total_bars) { object_error((t_object *)x, "compas: indice debe ser menor al total de compases: %d", total_bars); return; }
		do_add_bar(x, index);
	}
	//object_post((t_object *)x, "nuevo compas agregado");
	fl_batuta_actualizar_notas(x);
}
void do_add_bar(t_fl_batuta *x, long pos)
{
	fl_bar *pbar = (fl_bar *)sysmem_newptr(sizeof(fl_bar));
	pbar->notas = linklist_new();
	linklist_flags(pbar->notas, OBJ_FLAG_MEMORY);
	linklist_insertindex(x->l_bars, pbar, pos);
	pbar->hdl_nota = (fl_nota **)sysmem_newptr(sizeof(fl_nota *));
}

void fl_batuta_borrar_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//1 arg: compas
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long index;

	if (x->onoff) { object_error((t_object *)x, "compas: no se puede editar en modo play"); return; }
	if (ac != 1) { object_error((t_object *)x, "compas: borrar compas recibe 1 elemento: n compas"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "compas: n compas debe ser entero"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	index = (long)atom_getlong(ap);

	if (!total_bars) { object_error((t_object *)x, "compas: no hay compases"); return; }
	if (index >= total_bars) { object_error((t_object *)x, "compas: indice debe ser menor al total de compases: %d", total_bars); return; }

	do_delete_bar(x, index);

	//object_post((t_object *)x, "compas borrado");
	fl_batuta_actualizar_notas(x);
}
void do_delete_bar(t_fl_batuta *x, long pos)
{
	fl_nota *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, pos);
	long n_notas = (long)linklist_getsize(pbar->notas);
	for (int j = 0; j < n_notas; j++) {
		pnota = linklist_getindex(pbar->notas, j);
		sysmem_freeptr(pnota->pnota); //free atom array
	}
	if (!pbar->hdl_nota) {
		sysmem_freeptr(pbar->hdl_nota);
	}
	object_free(pbar->notas); //free linked list de notas en compas

	linklist_deleteindex(x->l_bars, pos);
}

void fl_batuta_copiar_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//2 arg: index_orig index_dest
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long bar_orig;
	long bar_dest;

	if (x->onoff) { object_error((t_object *)x, "compas: no se puede editar en modo play"); return; }
	if (ac != 2) { object_error((t_object *)x, "compas: copiar compas recibe 2 elementos: n compas origen, n compas destino"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "compas: n compas origen debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "compas: n compas destino debe ser entero"); return; }

	bar_orig = (long)atom_getlong(ap);
	bar_dest = (long)atom_getlong(ap + 1);

	if (!total_bars) { object_error((t_object *)x, "compas: no hay compases"); return; }
	if (bar_orig >= total_bars) { object_error((t_object *)x, "compas: n compas origen debe ser menor a total de compases"); return; }
	if (bar_dest >= total_bars) { object_error((t_object *)x, "compas: n compas destino debe ser menor a total de compases"); return; }

	do_copy_bar(x, bar_orig, bar_dest);

	//object_post((t_object *)x, "compas copiado");
	fl_batuta_actualizar_notas(x);
}
void do_copy_bar(t_fl_batuta *x, long orig, long dest)
{
	long n_notas;
	fl_nota *pnota_orig;
	fl_nota *pnota_dest;
	fl_bar *pbar_orig = linklist_getindex(x->l_bars, orig);
	fl_bar *pbar_dest = (fl_bar *)sysmem_newptr(sizeof(fl_bar));

	pbar_dest->notas = linklist_new();
	linklist_flags(pbar_dest->notas, OBJ_FLAG_MEMORY);
	pbar_dest->hdl_nota = (fl_nota **)sysmem_newptr(sizeof(fl_nota *));

	n_notas = (long)linklist_getsize(pbar_orig->notas);
	for (int j = 0; j < n_notas; j++) {
		pnota_orig = linklist_getindex(pbar_orig->notas, j);
		pnota_dest = (fl_nota *)sysmem_newptr(sizeof(fl_nota));
		pnota_dest->cnota = pnota_orig->cnota;
		pnota_dest->canal = pnota_orig->canal;
		pnota_dest->b_inicio = pnota_orig->b_inicio;
		pnota_dest->pnota = (t_atom *)sysmem_newptr((pnota_orig->cnota) * sizeof(t_atom));
		atom_getatom_array(pnota_orig->cnota, pnota_orig->pnota, pnota_dest->cnota, pnota_dest->pnota);
		linklist_insertindex(pbar_dest->notas, pnota_dest, j);
	}
	linklist_insertindex(x->l_bars, pbar_dest, dest);
}

void fl_batuta_mover_compas(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//2 arg: index_orig index_dest
	t_atom *ap = argv;
	long ac = argc;
	long bar_dest;
	long bar_orig;
	long total_bars;

	if (x->onoff) { object_error((t_object *)x, "compas: no se puede editar en modo play"); return; }
	if (ac != 2) { object_error((t_object *)x, "compas: copiar compas recibe 2 elementos: n compas origen, n compas destino"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "compas: n compas origen debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "compas: n compas destino debe ser entero"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	bar_orig = (long)atom_getlong(ap);
	bar_dest = (long)atom_getlong(ap + 1);

	if (!total_bars) { object_error((t_object *)x, "compas: no hay compases"); return; }
	if (bar_orig >= total_bars) { object_error((t_object *)x, "compas: n compas origen debe ser menor a total de compases"); return; }
	if (bar_dest >= total_bars - 1) { object_error((t_object *)x, "compas: n compas destino debe ser menor a total de compases - 1"); return; }
	if (bar_orig == total_bars) { object_error((t_object *)x, "compas: compas origen debe ser distinto al compas de destino"); return; }

	do_move_bar(x, bar_orig, bar_dest);

	//object_post((t_object *)x, "compas cambiado");
	fl_batuta_actualizar_notas(x);
}
void do_move_bar(t_fl_batuta *x, long orig, long dest)
{
	fl_bar *pbar_orig = linklist_getindex(x->l_bars, orig);
	fl_bar *pbar_dest = linklist_getindex(x->l_bars, dest);
	if (orig > dest) {
		linklist_movebeforeobjptr(x->l_bars, pbar_orig, pbar_dest);
	}
	else if (orig < dest) {
		linklist_moveafterobjptr(x->l_bars, pbar_orig, pbar_dest);
	}
}
//------------
void fl_batuta_borrar_canal(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//2 arg: compas y canal
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_bar;
	long n_canal;

	if (x->onoff) { object_error((t_object *)x, "canal: no se puede editar en modo play"); return; }
	if (ac != 2) { object_error((t_object *)x, "canal: borrar canal recibe 2 elementos: n compas, n canal"); return; }

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "canal: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "canal: n canal debe ser entero"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	n_bar = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);

	if (n_bar >= total_bars) { object_error((t_object *)x, "canal: n compas debe ser menor a total de compases"); return; }

	do_delete_chan(x, n_bar, n_canal);

	//object_post((t_object *)x, "canal borrado");
	fl_batuta_actualizar_notas(x);
}
void do_delete_chan(t_fl_batuta *x, long bar, long chan) {
	fl_nota *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	long total_notas = (long)linklist_getsize(pbar->notas);
	for (int i = total_notas - 1; i >= 0; i--) {
		pnota = linklist_getindex(pbar->notas, i);
		if (pnota->canal == chan) {
			sysmem_freeptr(pnota->pnota);
			linklist_deleteindex(pbar->notas, i);
		}
	}
}

void fl_batuta_copiar_canal(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//3arg: compas, canal y compas llegada
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_bar_orig;
	long n_bar_dest;
	long n_canal;

	if (x->onoff) { object_error((t_object *)x, "canal: no se puede editar en modo play"); return; }
	if (ac != 3) { object_error((t_object *)x, "canal: copiar canal recibe 3 elementos: n compas, n canal, compas destino"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "canal: n compas de origen debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "canal: n canal debe ser entero"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "canal: n compas destino debe ser entero"); return; }

	n_bar_orig = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	n_bar_dest = (long)atom_getlong(ap + 2);

	if (n_bar_orig >= total_bars) { object_error((t_object *)x, "canal: n compas origen debe ser menor a total de compases"); return; }
	if (n_bar_dest >= total_bars) { object_error((t_object *)x, "canal: n compas destino debe ser menor a total de compases"); return; }

	do_copy_chan(x, n_canal, n_bar_orig, n_bar_dest);

	//object_post((t_object *)x, "canal copiado");
	fl_batuta_actualizar_notas(x);
}
void do_copy_chan(t_fl_batuta *x, long chan, long bar_orig, long bar_dest) {
	fl_nota *pnota_orig, *pnota_dest;
	fl_bar *pbar_orig = linklist_getindex(x->l_bars, bar_orig);
	fl_bar *pbar_dest = linklist_getindex(x->l_bars, bar_dest);
	long total_notas = (long)linklist_getsize(pbar_orig->notas);
	for (int j = 0; j < total_notas; j++) {
		pnota_orig = linklist_getindex(pbar_orig->notas, j);
		if (pnota_orig->canal == chan) {
			pnota_dest = (fl_nota *)sysmem_newptr(sizeof(fl_nota));
			pnota_dest->b_inicio = pnota_orig->b_inicio;
			pnota_dest->cnota = pnota_orig->cnota;
			pnota_dest->canal = pnota_orig->canal;
			pnota_dest->pnota = (t_atom *)sysmem_newptr((pnota_orig->cnota) * sizeof(t_atom));
			atom_getatom_array(pnota_orig->cnota, pnota_orig->pnota, pnota_dest->cnota, pnota_dest->pnota);
			linklist_insertindex(pbar_dest->notas, pnota_dest, 0);
		}
	}
}

void fl_batuta_editar_canal(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//3arg: compas, canal y numero canal
	//busca canal en compas y cambia el numero de canal y canal en cada nota
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal_orig;
	long n_canal_dest;

	if (x->onoff) { object_error((t_object *)x, "canal: no se puede editar en modo play"); return; }
	if (ac != 3) { object_error((t_object *)x, "canal: editar canal recibe 3 elementos: n compas, n canal, nuevo n canal"); return; }

	total_bars = (long)linklist_getsize(x->l_bars);

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "canal: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "canal: n canal debe ser entero"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "canal: n canal debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal_orig = (long)atom_getlong(ap + 1);
	n_canal_dest = (long)atom_getlong(ap + 2);

	if (n_compas >= total_bars) { object_error((t_object *)x, "canal: n compas debe ser menor a total de compases"); return; }

	do_edit_chan(x, n_compas, n_canal_orig, n_canal_dest);

	//object_post((t_object *)x, "canal editado");
	fl_batuta_actualizar_notas(x);
}
void do_edit_chan(t_fl_batuta *x, long bar, long chan_ini, long chan_fin)
{
	fl_nota *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	long total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "canal: no hay notas en este compas"); return; }

	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (pnota->canal == chan_ini) {
			pnota->canal = chan_fin;
			atom_setlong(pnota->pnota, chan_fin);
		}
	}
}
//------------
void fl_batuta_nueva_nota(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//min 4 arg: compas; inicio; canal; info... 
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal;
	float b_inicio;
	t_max_err err;

	if (x->onoff) { object_error((t_object *)x, "nota: no se puede editar en modo play"); return; }
	if (ac < 4) { object_error((t_object *)x, "nota: nueva nota recibe minimo 4 elementos: n compas, n canal, inicio, info..."); return; }

	total_bars = (long)linklist_getsize(x->l_bars);
	if (total_bars == 0) { object_error((t_object *)x, "primero crear compas"); return; }

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "nota: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_FLOAT && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "nota: inicio debe ser float"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "nota: n canal debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);
	b_inicio = (float)atom_getfloat(ap + 1);
	n_canal = (long)atom_getlong(ap + 2);

	if (n_compas >= total_bars) { object_error((t_object *)x, "nota: n compas debe ser menor a total de compases"); return; }

	err = do_add_note(x, n_compas, b_inicio, n_canal, ac - 3, ap + 3);

	//object_post((t_object *)x, "nueva nota agregada");
	if (!err) { fl_batuta_actualizar_notas(x); }

}
t_max_err do_add_note(t_fl_batuta *x, long bar, float inicio, long canal, long listac, t_atom *listav) {
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	fl_nota *pnota = (fl_nota *)sysmem_newptr(sizeof(fl_nota));
	if (!pnota) { return MAX_ERR_OUT_OF_MEM; }
	pnota->b_inicio = inicio;
	pnota->canal = canal;
	pnota->pnota = (t_atom *)sysmem_newptr((listac + 1) * sizeof(t_atom));
	if (!pnota->pnota) { return MAX_ERR_OUT_OF_MEM; }
	pnota->cnota = listac + 1;
	atom_setlong(pnota->pnota, canal);
	atom_getatom_array(listac, listav, pnota->cnota - 1, pnota->pnota + 1);
	linklist_insertindex(pbar->notas, pnota, 0);
	return MAX_ERR_NONE;
}

void fl_batuta_borrar_nota(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//3 arg: compas; canal; pos_nota
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal;
	long index_borrar;
	long total_notas;
	long index = -1;
	fl_nota *pnota;
	fl_bar *pbar;
	t_max_err err;

	if (x->onoff) { object_error((t_object *)x, "nota: no se puede editar en modo play"); return; }
	if (ac != 3) { object_error((t_object *)x, "nota: borrar nota recibe 3 elementos: n compas, n canal, index nota"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "nota: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "nota: n canal debe ser entero"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "nota: index nota debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	index_borrar = (long)atom_getlong(ap + 2);
	total_bars = (long)linklist_getsize(x->l_bars);

	if (n_compas >= total_bars) { object_error((t_object *)x, "nota: n compas debe ser menor a total de compases"); return; }

	pbar = linklist_getindex(x->l_bars, n_compas);
	total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "nota: no hay notas"); return; }
	if (index_borrar >= total_notas && index_borrar < 0) { object_error((t_object *)x, "nota: index nota debe ser menor a total de notas en canal"); return; }

	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (pnota->canal == n_canal) {
			index++;
		}
	}
	if (index == -1) { object_error((t_object *)x, "nota: no se encontró canal en compas"); return; }
	if (index < index_borrar) { object_error((t_object *)x, "nota: no se encontró index en compas"); return; }

	err = do_delete_note(x, n_compas, index_borrar);

	//object_post((t_object *)x, "nota borrada");
	if (!err) { fl_batuta_actualizar_notas(x); }
}
t_max_err do_delete_note(t_fl_batuta *x, long bar, long index_nota) {
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);
	long total_notas = (long)linklist_getsize(pbar->notas);
	if (index_nota >= total_notas || index_nota < 0) { return MAX_ERR_GENERIC; }
	linklist_deleteindex(pbar->notas, index_nota);
	return MAX_ERR_NONE;
}

void fl_batuta_editar_nota(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//min 3 arg: compas; canal; pos_nota; info... 
	t_atom *ap = argv;
	long ac = argc;
	long total_bars;
	long n_compas;
	long n_canal;
	long index_editar;
	long total_notas;
	long index = -1;
	fl_bar *pbar;
	fl_nota *pnota;
	t_max_err err;

	if (x->onoff) { object_error((t_object *)x, "nota: no se puede editar en modo play"); return; }
	if (ac < 4) { object_error((t_object *)x, "nota: editar nota recibe minimo 4 elementos: n compas, n canal, index nota, info..."); return; }

	total_bars = (long)linklist_getsize(x->l_bars);

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "nota: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "nota: n canal debe ser entero"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "nota: index nota debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	index_editar = (long)atom_getlong(ap + 2);

	if (n_compas >= total_bars) { object_error((t_object *)x, "nota: n compas debe ser menor a total de compases"); return; }

	pbar = linklist_getindex(x->l_bars, n_compas);
	total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "nota: no hay notas en este canal"); return; }

	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (pnota->canal == n_canal) {
			index++;
		}
		if (index == index_editar) {
			break;
		}
	}
	if (index == -1) { object_error((t_object *)x, "nota: no se encontró canal en compas"); return; }
	if (index_editar > index) { object_error((t_object *)x, "nota: no se encontró index en compas"); return; }

	err = do_edit_note_info(x, n_compas, index, ac - 3, ap + 3);

	//object_post((t_object *)x, "nota editada");
	if (!err) { fl_batuta_actualizar_notas(x); }
}
t_max_err do_edit_note_info(t_fl_batuta *x, long bar, long index_nota, long listac, t_atom *listav) {
	fl_nota *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);

	long total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "nota: no hay notas"); return MAX_ERR_GENERIC; }
	if (index_nota >= total_notas && index_nota < 0) { object_error((t_object *)x, "nota: index nota debe ser menor a total de notas en canal"); return MAX_ERR_GENERIC; }

	pnota = linklist_getindex(pbar->notas, index_nota);
	pnota->pnota = (t_atom *)sysmem_resizeptr(pnota->pnota, listac * sizeof(t_atom));
	pnota->cnota = listac;
	atom_setlong(pnota->pnota, pnota->canal);
	atom_getatom_array(listac, listav, pnota->cnota - 1, pnota->pnota + 1);

	return MAX_ERR_NONE;
}
t_max_err do_edit_note_inicio(t_fl_batuta *x, long bar, long index_nota, float inicio) {
	fl_nota *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);

	long total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "nota: no hay notas"); return MAX_ERR_GENERIC; }
	if (index_nota >= total_notas && index_nota < 0) { object_error((t_object *)x, "nota: index nota debe ser menor a total de notas en canal"); return MAX_ERR_GENERIC; }

	pnota = linklist_getindex(pbar->notas, index_nota);
	pnota->b_inicio = inicio;
	return MAX_ERR_NONE;
}
t_max_err do_edit_note_canal(t_fl_batuta *x, long bar, long index_nota, long canal) {
	fl_nota *pnota;
	fl_bar *pbar = linklist_getindex(x->l_bars, bar);

	long total_notas = (long)linklist_getsize(pbar->notas);
	if (!total_notas) { object_error((t_object *)x, "nota: no hay notas"); return MAX_ERR_GENERIC; }
	if (index_nota >= total_notas && index_nota < 0) { object_error((t_object *)x, "nota: index nota debe ser menor a total de notas en canal"); return MAX_ERR_GENERIC; }

	pnota = linklist_getindex(pbar->notas, index_nota);
	pnota->canal = canal;
	atom_setlong(pnota->pnota, canal);
	return MAX_ERR_NONE;
}
//------------
void fl_batuta_nueva_cifra(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//3 arg: n compas; num; den
	t_atom *ap = argv;
	long ac = argc;
	float num;
	long den;
	long n_bar;

	if (x->onoff) { object_error((t_object *)x, "cifra: no se puede editar en modo play"); return; }
	if (ac != 3) { object_error((t_object *)x, "cifra: nueva cifra recibe 3 elementos: n compas, numerador; denominador"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "cifra: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_FLOAT && atom_gettype(ap + 1) != A_LONG) { object_error((t_object *)x, "cifra: numerador debe ser float"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "cifra: denominador debe ser entero"); return; }

	n_bar = (long)atom_getlong(ap);
	num = (float)atom_getfloat(ap + 1);
	den = (long)atom_getlong(ap + 2);

	if (num <= 0) { object_error((t_object *)x, "cifra: numerador debe ser positivo mayor que cero"); return; }
	if (den <= 0) { object_error((t_object *)x, "cifra: denominador no puede ser cero"); return; }

	do_add_cifra(x, n_bar, num, den);

	//object_post((t_object *)x, "nueva cifra agregada");
	fl_batuta_actualizar_cifras(x);
}
void do_add_cifra(t_fl_batuta *x, long bar, float numer, long denom) {
	long total_cifras, index;
	fl_cifra *foundcifra = NULL;
	fl_cifra *pcifra;

	total_cifras = (long)linklist_getsize(x->l_cifras);

	pcifra = (fl_cifra *)sysmem_newptr(sizeof(fl_cifra));
	pcifra->n_bar = bar;

	if (total_cifras) { index = (long)linklist_findfirst(x->l_cifras, &foundcifra, cifra_mismocompas, pcifra); }
	else { index = -1; }

	if (index != -1) {
		foundcifra->negras = (float)(4.0 * numer / (float)denom);
		sysmem_freeptr(pcifra);
	}
	else {
		pcifra->negras = (float)(4.0 * numer / (float)denom);
		linklist_insertindex(x->l_cifras, pcifra, 0);
		linklist_sort(x->l_cifras, cifra_compasmenor);
	}
}

void fl_batuta_borrar_cifra(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	t_atom *ap = argv;
	long ac = argc;
	long n_compas;

	if (x->onoff) { object_error((t_object *)x, "cifra: no se puede editar en modo play"); return; }
	if (ac != 1) { object_error((t_object *)x, "cifra: borrar cifra recibe 1 elemento: n compas"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "cifra: n compas debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);

	do_delete_cifra(x, n_compas);

	//object_post((t_object *)x, "cifra borrada");
	fl_batuta_actualizar_cifras(x);
}
void do_delete_cifra(t_fl_batuta *x, long bar) {
	long index, total_cifras;
	fl_cifra *pcifra, *foundcifra;
	total_cifras = (long)linklist_getsize(x->l_cifras);
	if (total_cifras) {
		pcifra = (fl_cifra *)sysmem_newptr(sizeof(fl_cifra));
		pcifra->n_bar = bar;
		index = (long)linklist_findfirst(x->l_cifras, &foundcifra, cifra_mismocompas, pcifra);
		sysmem_freeptr(pcifra);

		if (index == -1) { object_error((t_object *)x, "cifra: no se encontró cifra en compas"); return; }
		linklist_deleteindex(x->l_cifras, index);
	}
}

//------------
void fl_batuta_nuevo_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//3 arg: n compas; ms inicio; ms beat						//0:cambio inmediato
	//4 arg: n compas; ms inicio; ms beat; durac dtempo			//1:cambio formato line
	//5 arg: n compas; ms inicio; ms beat; durac dtempo; curva	//2:cambio formato curve
	t_atom *ap = argv;
	long ac = argc;
	short type;
	long n_bar;
	float ms_inicio, ms_beat, ms_durvar, curva;

	if (x->onoff) { object_error((t_object *)x, "tempo: no se puede editar en modo play"); return; }
	if (ac < 3 || ac > 5) { object_error((t_object *)x, "tempo: nuevo tempo recibe 3 a 5 elementos: n compas, ms delay cambio, ms beat, (duracion dtempo, curva)"); return; }

	type = (short)ac - 3;

	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "tempo: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_FLOAT && atom_gettype(ap + 1) != A_LONG) { object_error((t_object *)x, "tempo: ms delay inicio de cambio debe ser float"); return; }
	if (atom_gettype(ap + 2) != A_FLOAT && atom_gettype(ap + 2) != A_LONG) { object_error((t_object *)x, "tempo: ms beat (periodo) debe ser float"); return; }
	n_bar = (long)atom_getlong(ap);
	ms_inicio = (float)atom_getfloat(ap + 1);
	ms_beat = (float)atom_getfloat(ap + 2);
	if (type > 0) {
		if (atom_gettype(ap + 3) != A_FLOAT && atom_gettype(ap + 3) != A_LONG) { object_error((t_object *)x, "tempo: ms duracion cambio debe ser float"); return; }
		ms_durvar = (float)atom_getfloat(ap + 3);

		if (type == 2) {
			if (atom_gettype(ap + 4) != A_FLOAT && atom_gettype(ap + 4) != A_LONG) { object_error((t_object *)x, "tempo: curva debe ser float"); return; }
			curva = parse_curve((float)atom_getfloat(ap + 4));
		}
		else {
			curva = 0.5;
		}
	}
	else {
		ms_durvar = 0.0;
		curva = 0.5;
	}

	do_add_tempo(x, type, n_bar, ms_inicio, ms_beat, ms_durvar, curva);

	//object_post((t_object *)x, "nuevo tempo agregado");
	fl_batuta_actualizar_tempos(x);
}
void do_add_tempo(t_fl_batuta *x, short tipo, long bar, float inicio, float msbeat, float var, float cur) {
	long total_tempos, index;
	fl_tempo *foundtempo = NULL;
	fl_tempo *ptempo;

	total_tempos = (long)linklist_getsize(x->l_tempos);

	ptempo = (fl_tempo *)sysmem_newptr(sizeof(fl_tempo));
	ptempo->n_bar = bar;

	if (total_tempos) { index = (long)linklist_findfirst(x->l_tempos, &foundtempo, tempo_mismocompas, ptempo); }
	else { index = -1; }

	if (index != -1) {
		sysmem_freeptr(ptempo);

		foundtempo->type = tipo;
		foundtempo->ms_inicio = inicio;
		foundtempo->ms_beat = msbeat;
		foundtempo->ms_durvar = var;
		foundtempo->curva = cur;
	}
	else {
		ptempo->type = tipo;
		ptempo->ms_inicio = inicio;
		ptempo->ms_beat = msbeat;
		ptempo->ms_durvar = var;
		ptempo->curva = cur;

		linklist_insertindex(x->l_tempos, ptempo, 0);
		linklist_sort(x->l_tempos, tempo_compasmenor);
	}
}

void fl_batuta_borrar_tempo(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//1 arg: n compas;
	t_atom *ap = argv;
	long ac = argc;
	long n_compas;

	if (x->onoff) { object_error((t_object *)x, "tempo: no se puede editar en modo play"); return; }
	if (ac != 1) { object_error((t_object *)x, "tempo: borrar tempo recibe 1 elemento: n compas"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "tempo: n compas debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);

	do_delete_tempo(x, n_compas);

	//object_post((t_object *)x, "tempo borrado");
	fl_batuta_actualizar_tempos(x);
}
void do_delete_tempo(t_fl_batuta *x, long bar) {
	long index, total_tempos;
	fl_tempo *foundtempo, *ptempo;

	total_tempos = (long)linklist_getsize(x->l_tempos);
	if (total_tempos) {
		ptempo = (fl_tempo *)sysmem_newptr(sizeof(fl_tempo));
		ptempo->n_bar = bar;
		index = (long)linklist_findfirst(x->l_tempos, &foundtempo, tempo_mismocompas, ptempo);
		sysmem_freeptr(ptempo);

		if (index == -1) { object_error((t_object *)x, "tempo: no se encontró tempo en compas"); return; }

		linklist_deleteindex(x->l_tempos, index);
	}
}
//------------

void fl_batuta_nuevo_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//3 arg: n compas; n compas dest; n repet
	t_atom *ap = argv;
	long ac = argc;
	long n_bar, total_rep, to_bar;

	if (x->onoff) { object_error((t_object *)x, "goto: no se puede editar en modo play"); return; }
	if (ac != 3) { object_error((t_object *)x, "goto: nuevo goto recibe 3 elementos: n compas, compas destino; n repeticiones"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "goto: n compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "goto: compas destino debe ser entero"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "goto: n repeticiones debe ser entero"); return; }

	n_bar = (long)atom_getlong(ap);
	to_bar = (long)atom_getlong(ap + 1);
	total_rep = (long)atom_getlong(ap + 2);

	do_add_goto(x, n_bar, to_bar, total_rep);

	//object_post((t_object *)x, "nuevo goto agregado");
	fl_batuta_actualizar_gotos(x);
}
void do_add_goto(t_fl_batuta *x, long bar_orig, long bar_dest, long repet) {
	fl_goto *pgoto;
	fl_goto *foundgoto = NULL;
	long index, total_gotos;

	total_gotos = (long)linklist_getsize(x->l_gotos);

	pgoto = (fl_goto *)sysmem_newptr(sizeof(fl_goto));
	pgoto->n_bar = bar_orig;

	if (total_gotos) { index = (long)linklist_findfirst(x->l_gotos, &foundgoto, goto_mismocompas, pgoto); }
	else { index = -1; }

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

		linklist_insertindex(x->l_gotos, pgoto, 0);
		linklist_sort(x->l_gotos, goto_compasmenor);
	}
}

void fl_batuta_borrar_goto(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//1 arg: n compas
	t_atom *ap = argv;
	long ac = argc;
	long n_compas;

	if (x->onoff) { object_error((t_object *)x, "goto: no se puede editar en modo play"); return; }
	if (ac != 1) { object_error((t_object *)x, "goto: borrar goto recibe 1 elemento: n compas"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "goto: n compas debe ser entero"); return; }

	n_compas = (long)atom_getlong(ap);

	do_delete_goto(x, n_compas);

	//object_post((t_object *)x, "goto borrado");
	fl_batuta_actualizar_gotos(x);
}
void do_delete_goto(t_fl_batuta *x, long bar) {
	fl_goto *pgoto, *foundgoto;
	long total_gotos, index;

	total_gotos = (long)linklist_getsize(x->l_gotos);
	if (total_gotos) {
		pgoto = (fl_goto *)sysmem_newptr(sizeof(fl_goto));
		pgoto->n_bar = bar;
		index = (long)linklist_findfirst(x->l_gotos, &foundgoto, goto_mismocompas, pgoto);
		sysmem_freeptr(pgoto);

		if (index == -1) { object_error((t_object *)x, "goto: no se encontró goto en compas"); return; }

		linklist_deleteindex(x->l_gotos, index);
	}
}


void fl_batuta_quantize_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	//recibe: bar, chan, division de negra
	t_atom *ap = argv;
	long ac = argc;
	long n_compas, n_canal;
	float div;

	if (x->onoff) { object_error((t_object *)x, "quant: solo durante modo edicion"); return; }
	if (ac != 3 || !ap) { object_error((t_object *)x, "quantize recibe 3 elementos: compas, canal, divisiones de negra"); return; }
	if (atom_gettype(ap) != A_FLOAT && atom_gettype(ap) != A_LONG) { object_error((t_object *)x, "quant: compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_FLOAT && atom_gettype(ap + 1) != A_LONG) { object_error((t_object *)x, "quant: canal debe ser entero"); return; }
	if (atom_gettype(ap + 2) != A_FLOAT && atom_gettype(ap + 2) != A_LONG) { object_error((t_object *)x, "quant: division de negra debe ser numero"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	div = (float)atom_getfloat(ap + 2);

	do_quantize_chan(x, n_compas, n_canal, div);

	fl_batuta_actualizar_notas(x);
}
void do_quantize_chan(t_fl_batuta *x, long bar, long chan, float div)
{
	long total_bars, total_notas, total_cifras;
	fl_bar *pbar, *pbar_new;
	fl_nota *pnota;
	fl_cifra *pcifra;
	long index_cifrafound, mayor_cifrafound;
	float cifra_found;

	total_bars = (long)linklist_getsize(x->l_bars);
	if (bar < 0 || bar >= total_bars) { object_error((t_object *)x, "quant: no existe compas"); return; }
	if (div < 1) { object_error((t_object *)x, "quant: numero divisiones debe ser mayor o igual a 1"); return; }

	pbar = linklist_getindex(x->l_bars, bar);

	//cifra
	mayor_cifrafound = -1;
	index_cifrafound = -1;
	total_cifras = (long)linklist_getsize(x->l_cifras);
	for (int i = 0; i < total_cifras; i++) {
		pcifra = linklist_getindex(x->l_cifras, i);
		if (pcifra->n_bar >= mayor_cifrafound && pcifra->n_bar <= bar) {
			mayor_cifrafound = pcifra->n_bar;
			index_cifrafound = i;
		}
	}

	if (index_cifrafound == -1) { object_error((t_object *)x, "quant: no se encontró cifra"); return; }

	pcifra = linklist_getindex(x->l_cifras, index_cifrafound);
	cifra_found = pcifra->negras;

	//quant
	total_notas = (long)linklist_getsize(pbar->notas);
	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);

		if (pnota->canal == chan) {
			pnota->b_inicio = (float)round((double)pnota->b_inicio * (double)div) / (float)div;

			//b_inicio se cuantizó a final de compas
			if (pnota->b_inicio >= cifra_found) {
				pnota->b_inicio -= cifra_found;

				linklist_chuckindex(pbar->notas, i);

				//compas es el ultimo
				if (bar == total_bars - 1) {
					pbar_new = (fl_bar *)sysmem_newptr(sizeof(fl_bar));
					pbar_new->notas = linklist_new();
					linklist_flags(pbar_new->notas, OBJ_FLAG_MEMORY);
				}
				else {
					pbar = linklist_getindex(x->l_bars, bar + 1);
				}

				linklist_insertindex(pbar->notas, pnota, 0);
				linklist_sort(pbar->notas, nota_iniciomenor);
			}
		}
	}
}

void fl_batuta_human_chan(t_fl_batuta *x, t_symbol *s, long argc, t_atom *argv)
{
	float margen_fijo, margen_random_max;
	t_atom *ap = argv;
	long ac = argc;
	long n_compas, n_canal;

	if (x->onoff) { object_error((t_object *)x, "human: solo durante modo edicion"); return; }
	if (argc != 4) { object_error((t_object *)x, "human: acepta 4 argumentos: compas, canal, (+/-)margen fijo(%%beat), (+/-)margen aleatorio max(%%beat)"); return; }
	if (atom_gettype(ap) != A_LONG && atom_gettype(ap) != A_FLOAT) { object_error((t_object *)x, "human: compas debe ser entero"); return; }
	if (atom_gettype(ap + 1) != A_LONG && atom_gettype(ap + 1) != A_FLOAT) { object_error((t_object *)x, "human: canal debe ser entero"); return; }
	if (atom_gettype(ap + 2) != A_LONG && atom_gettype(ap + 2) != A_FLOAT) { object_error((t_object *)x, "human: margen fijo debe ser float"); return; }
	if (atom_gettype(ap + 3) != A_LONG && atom_gettype(ap + 3) != A_FLOAT) { object_error((t_object *)x, "human: margen aleatorio debe ser float"); return; }

	n_compas = (long)atom_getlong(ap);
	n_canal = (long)atom_getlong(ap + 1);
	margen_fijo = (float)(atom_getfloat(ap + 2) / 100.0);
	margen_random_max = (float)(atom_getfloat(ap + 3) / 100.0);

	do_human_chan(x, n_compas, n_canal, margen_fijo, margen_random_max);

	fl_batuta_actualizar_notas(x);
}
void do_human_chan(t_fl_batuta *x, long bar, long chan, float d_fijo, float d_random)
{
	long total_bars, total_notas;
	fl_bar *pbar, *pbar_new;
	fl_nota *pnota;
	long compasmayor_cifrafound, indexcompas_cifrafound, total_cifras;
	float cifra_compas_ant, cifra_compas;
	fl_cifra *pcifra;
	float margen_random, b_inicio;

	total_bars = (long)linklist_getsize(x->l_bars);
	if (bar < 0 || bar >= total_bars) { object_error((t_object *)x, "human: no existe compas"); return; }

	pbar = linklist_getindex(x->l_bars, bar);

	//cifra
	compasmayor_cifrafound = -1;
	indexcompas_cifrafound = -1;
	total_cifras = (long)linklist_getsize(x->l_cifras);
	for (int i = 0; i < total_cifras; i++) {
		pcifra = linklist_getindex(x->l_cifras, i);
		if (pcifra->n_bar >= compasmayor_cifrafound && pcifra->n_bar <= bar) {
			compasmayor_cifrafound = pcifra->n_bar;
			indexcompas_cifrafound = i;
		}
	}
	if (indexcompas_cifrafound == -1) { object_error((t_object *)x, "human: no se encontró cifra compas"); return; }
	pcifra = linklist_getindex(x->l_cifras, indexcompas_cifrafound);
	cifra_compas = pcifra->negras;

	//cifra compas anterior
	if (bar) {
		compasmayor_cifrafound = -1;
		indexcompas_cifrafound = -1;
		total_cifras = (long)linklist_getsize(x->l_cifras);
		for (int i = 0; i < total_cifras; i++) {
			pcifra = linklist_getindex(x->l_cifras, i);
			if (pcifra->n_bar >= compasmayor_cifrafound && pcifra->n_bar <= bar) {
				compasmayor_cifrafound = pcifra->n_bar;
				indexcompas_cifrafound = i;
			}
		}
		if (indexcompas_cifrafound == -1) { object_error((t_object *)x, "human: no se encontró cifra compas anterior"); return; }
		pcifra = linklist_getindex(x->l_cifras, indexcompas_cifrafound);
		cifra_compas_ant = pcifra->negras;
	}

	total_notas = (long)linklist_getsize(pbar->notas);
	for (int i = 0; i < total_notas; i++) {
		pnota = linklist_getindex(pbar->notas, i);
		if (chan == pnota->canal) {

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

					linklist_chuckindex(pbar->notas, i);
					pbar = linklist_getindex(x->l_bars, bar - 1);
					linklist_insertindex(pbar->notas, pnota, 0);
				}
			}
			else if (b_inicio >= cifra_compas) {
				b_inicio -= cifra_compas;
				pnota->b_inicio = b_inicio;

				linklist_chuckindex(pbar->notas, i);

				if (bar == total_bars - 1) {
					pbar_new = (fl_bar *)sysmem_newptr(sizeof(fl_bar));
					pbar_new->notas = linklist_new();
					linklist_flags(pbar_new->notas, OBJ_FLAG_MEMORY);
					linklist_insertindex(x->l_bars, pbar_new, total_bars);
				}
				else {
					pbar = linklist_getindex(x->l_bars, bar + 1);
				}
				linklist_insertindex(pbar->notas, pnota, 0);
			}
			else {
				pnota->b_inicio = b_inicio;
			}
		}
	}
}
