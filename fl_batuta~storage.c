#include "fl_batuta~.h"

/*almacenar---------------------------------------------------------------------------------------------*/
void fl_batuta_read(t_fl_batuta *x, t_symbol *s)
{
	if (!(x->onoff)) {
		defer(x, (method)fl_batuta_doread, s, 0, NULL);
	}
}

void fl_batuta_doread(t_fl_batuta *x, t_symbol *s)
{
	t_fourcc filetype = 'TEXT', outtype;
	short numtypes = 1;
	char filename[MAX_PATH_CHARS];
	short path;
	if (s == gensym("")) {      // if no argument supplied, ask for file
		if (open_dialog(filename, &path, &outtype, &filetype, 1))       // non-zero: user cancelled
			return;
	}
	else {
		strcpy(filename, s->s_name);    // must copy symbol before calling locatefile_extended
		if (locatefile_extended(filename, &path, &outtype, &filetype, 1)) { // non-zero: not found
			object_error((t_object *)x, "%s: not found", s->s_name);
			return;
		}
	}
	// we have a file
	object_post((t_object *)x, "archivo %s encontrado", filename);
	fl_batuta_openfile(x, filename, path);
}

void fl_batuta_openfile(t_fl_batuta *x, char *filename, short path)
{
	long total_bars;
	fl_bar *pbar;
	long total_notas;
	fl_nota *pnota;

	t_filehandle fh;
	char **texthandle;
	long accum;
	char *linea = (char *)sysmem_newptr(LARGO_MAX_LINEA * sizeof(char));
	t_atom *av = (t_atom *)sysmem_newptr(100 * sizeof(t_atom));
	//t_atom *av = NULL;
	long ac = 0;
	//char flag = NULL;
	//atom_alloc_array(100, &ac, &av, &flag);
	t_max_err err = MAX_ERR_NONE;

	if (path_opensysfile(filename, path, &fh, READ_PERM)) {
		object_error((t_object *)x, "error opening %s", filename);
		return;
	}
	// allocate some empty memory to receive text
	texthandle = sysmem_newhandle(0);
	sysfile_readtextfile(fh, texthandle, 0, TEXT_NULL_TERMINATE);
	x->largo_texto = sysmem_handlesize(texthandle);
	//post("the file has %ld characters", x->largo_texto);

	//limpiar listas
	total_bars = (long)linklist_getsize(x->l_bars);
	for (int i = 0; i < total_bars; i++) {
		pbar = linklist_getindex(x->l_bars, i);
		total_notas = (long)linklist_getsize(pbar->notas);
		for (int k = 0; k < total_notas; k++) {
			pnota = linklist_getindex(pbar->notas, k);
			sysmem_freeptr(pnota->pnota);
		}
		object_free(pbar->notas);
	}
	linklist_clear(x->l_bars);

	linklist_clear(x->l_tempos);
	linklist_clear(x->l_cifras);
	linklist_clear(x->l_gotos);

	accum = 0;
	while (accum < x->largo_texto) {
		accum += getlinea(linea, *texthandle + accum, LARGO_MAX_LINEA);
		my_setparse(&ac, &av, linea);

		if (atom_gettype(av) == A_SYM) {
			if (atom_getsym(av) == gensym("compas")) {
				if (ac == 1) {
					fl_batuta_nuevo_compas(x, NULL, 0, NULL);
				}
				else if (ac == 2) {
					fl_batuta_nuevo_compas(x, NULL, ac - 1, av + 1);
				}
			}
			else if (atom_getsym(av) == gensym("nota")) {
				fl_batuta_nueva_nota(x, NULL, ac - 1, av + 1);
			}
			else if (atom_getsym(av) == gensym("cifra")) {
				fl_batuta_nueva_cifra(x, NULL, ac - 1, av + 1);
			}
			else if (atom_getsym(av) == gensym("tempo")) {
				fl_batuta_nuevo_tempo(x, NULL, ac - 1, av + 1);
			}
			else if (atom_getsym(av) == gensym("goto")) {
				fl_batuta_nuevo_goto(x, NULL, ac - 1, av + 1);
			}
		}
	}

	sysfile_close(fh);
	sysmem_freehandle(texthandle);
	sysmem_freeptr(linea);
	sysmem_freeptr(av);

	object_post((t_object *)x, "fin de lectura");
}

void fl_batuta_write(t_fl_batuta *x, t_symbol *s)
{
	defer(x, (method)fl_batuta_dowrite, s, 0, NULL);
}

void fl_batuta_dowrite(t_fl_batuta *x, t_symbol *s)
{
	t_fourcc filetype = 'TEXT', outtype;
	short numtypes = 1;
	char filename[MAX_FILENAME_CHARS];
	short path;

	strcpy(filename, "untitled");
	if (s == gensym("")) {      // if no argument supplied, ask for file
		//saveas_promptset("elije el archivo");
		if (saveasdialog_extended(filename, &path, &outtype, &filetype, numtypes)) {     // non-zero: user cancelled
			return;
		}
	}
	else {
		strcpy(filename, s->s_name);
		path = path_getdefault();
	}
	fl_batuta_writefile(x, filename, path);
}

void fl_batuta_writefile(t_fl_batuta *x, char *filename, short path)
{
	t_fourcc type = 'TEXT';
	long linea_len = 0;
	char *linea = (char *)sysmem_newptr(LARGO_MAX_LINEA * sizeof(char));
	t_handle h = sysmem_newhandle(0);
	t_max_err err;
	t_filehandle fh;
	fl_bar *pbar;
	long total_bars;
	fl_nota *pnota;
	long total_notas;
	t_atom a_nota[3];
	fl_cifra *pcifra;
	long total_cifras;
	t_atom a_cifra[3];
	fl_tempo *ptempo;
	long total_tempos;
	t_atom a_tempo[3];
	short tem_type;
	t_atom a_tempoaux[1];
	fl_goto *pgoto;
	long total_gotos;
	t_atom a_goto[3];

	total_bars = (long)linklist_getsize(x->l_bars);
	for (int i = 0; i < total_bars; i++) {
		sysmem_ptrandhand("compas\r\n", h, 8 * sizeof(char));
		pbar = linklist_getindex(x->l_bars, i);
		total_notas = (long)linklist_getsize(pbar->notas);
		for (int k = 0; k < total_notas; k++) {
			sysmem_ptrandhand("nota ", h, 5 * sizeof(char));
			pnota = linklist_getindex(pbar->notas, k);
			atom_setlong(a_nota, i);//compas
			atom_setfloat(a_nota + 1, pnota->b_inicio);//inicio
			atom_setlong(a_nota + 2, pnota->canal);//canal
			my_gettext(3, a_nota, &linea_len, &linea, 0);
			sysmem_ptrandhand(linea, h, (linea_len - 1) * sizeof(char));

			sysmem_ptrandhand(" ", h, sizeof(char));
			my_gettext((pnota->cnota) - 1, (pnota->pnota) + 1, &linea_len, &linea, 0);//info
			sysmem_ptrandhand(linea, h, (linea_len - 1) * sizeof(char));
			sysmem_ptrandhand("\r\n", h, 2 * sizeof(char));
		}
	}

	total_cifras = (long)linklist_getsize(x->l_cifras);
	for (int i = 0; i < total_cifras; i++) {
		sysmem_ptrandhand("cifra ", h, 6 * sizeof(char));

		pcifra = linklist_getindex(x->l_cifras, i);
		atom_setlong(a_cifra, pcifra->n_bar);//compas
		atom_setfloat(a_cifra + 1, pcifra->negras);//num
		atom_setlong(a_cifra + 2, 4);//den
		my_gettext(3, a_cifra, &linea_len, &linea, 0);
		sysmem_ptrandhand(linea, h, (linea_len - 1) * sizeof(char));
		sysmem_ptrandhand("\r\n", h, 2 * sizeof(char));
	}

	total_tempos = (long)linklist_getsize(x->l_tempos);
	for (int i = 0; i < total_tempos; i++) {
		sysmem_ptrandhand("tempo ", h, 6 * sizeof(char));

		ptempo = linklist_getindex(x->l_tempos, i);
		tem_type = ptempo->type;
		atom_setlong(a_tempo, ptempo->n_bar);//compas
		atom_setfloat(a_tempo + 1, ptempo->ms_inicio);//inicio
		atom_setfloat(a_tempo + 2, ptempo->ms_beat);//tempo
		my_gettext(3, a_tempo, &linea_len, &linea, 0);
		sysmem_ptrandhand(linea, h, (linea_len - 1) * sizeof(char));

		if (tem_type > 0) {
			sysmem_ptrandhand(" ", h, sizeof(char));
			atom_setfloat(a_tempoaux, ptempo->ms_durvar);//durac dtempo
			my_gettext(1, a_tempoaux, &linea_len, &linea, 0);
			sysmem_ptrandhand(linea, h, (linea_len - 1) * sizeof(char));
		}
		else if (tem_type == 2) {
			sysmem_ptrandhand(" ", h, sizeof(char));
			atom_setfloat(a_tempoaux, ptempo->curva);//curva
			my_gettext(1, a_tempoaux, &linea_len, &linea, 0);
			sysmem_ptrandhand(linea, h, (linea_len - 1) * sizeof(char));
		}
		sysmem_ptrandhand("\r\n", h, 2 * sizeof(char));
	}

	total_gotos = (long)linklist_getsize(x->l_gotos);
	for (int i = 0; i < total_gotos; i++) {
		sysmem_ptrandhand("goto ", h, 5 * sizeof(char));

		pgoto = linklist_getindex(x->l_gotos, i);
		atom_setlong(a_goto, pgoto->n_bar);
		atom_setlong(a_goto + 1, pgoto->to_bar);
		atom_setlong(a_goto + 2, pgoto->total_rep);
		my_gettext(3, a_goto, &linea_len, &linea, 0);
		sysmem_ptrandhand(linea, h, (linea_len - 1) * sizeof(char));
		sysmem_ptrandhand("\r\n", h, 2 * sizeof(char));
	}

	sysmem_ptrandhand("---", h, 4 * sizeof(char));

	err = path_createsysfile(filename, path, type, &fh);
	if (err) {
		return;
	}
	err = sysfile_writetextfile(fh, h, TEXT_LB_PC);

	sysfile_close(fh);
	sysmem_freehandle(h);
	sysmem_freeptr(linea);

	fl_batuta_actualizar(x);
}