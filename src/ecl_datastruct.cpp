#include <sstream>
#include "ecl_datastruct.h"


//****************//
// ecl_datastruct //
//****************//    

ecl_datastruct::ecl_datastruct()
{
	m_b_built = false;
	m_fname = "";
	m_n_rows = 0;
	m_n_vars = 0;
	m_last_tmp_save = 0;
}

ecl_datastruct::~ecl_datastruct()
{
	clear();
}

bool ecl_datastruct::build(long n_rows, enum vars)
{
	if(m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::build() - Data structure has already been built");
		return false;
	}

	if(n_rows < 2)
	{
		ecl_error("ECL: ecl_datastruct::build() - At least two rows needed");
		return false;
	}

	if(vN < 2)
	{
		ecl_error("ECL: ecl_datastruct::build() - At least two variables needed");
		return false;
	}

	m_n_rows = n_rows;
	m_n_vars = vN;
	m_data_matrix.reserve(m_n_rows);
	trial * row;

	for (int r=0; r<m_n_rows; r++)
	{			
		row = new trial;
		row->m_trial_vector.reserve(m_n_rows);
		
		for (int v=0; v<m_n_vars; v++)
			row->m_trial_vector.push_back(0);

		m_data_matrix.push_back(row);
	}
	
	m_b_built = true;
	return true;
}

bool ecl_datastruct::fill(long row_begin, long row_end, long var, string fillmethod, string vals)
{
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::fill() - Data structure has not been built");
		return false;
	}
	
	if (row_begin < 1 || row_begin > m_n_rows || row_end < 1 || row_end > m_n_rows)
	{
		ecl_error("ECL: ecl_datastruct::fill() - Invalid row number");
		return false;
	}

	if (row_begin >= row_end)
	{
		ecl_error("ECL: ecl_datastruct::fill() - Start row number should be smaller than the end row number");
		return false;
	}

	if (var < 0 || var >= m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::fill() - Variable does not exist");
		return false;
	}

	if (vals == "")
	{
		ecl_error("ECL: ecl_datastruct::fill() - Value string is empty");
		return false;
	}

	long n_row_subsp = row_end - row_begin + 1;
	vector<long> argvals;

	istringstream iss(vals);
	long n = 0;
	do
	{
		n++;

		if (n > n_row_subsp)
		{
			ecl_error("ECL: ecl_datastruct::fill() - Too many fill values");
			return false;
		}  		
			
		string sub;
		iss >> sub;
		argvals.push_back(strtol(sub.c_str(),NULL,1));
	} while (iss);


	// REPEAT METHOD //

	if (fillmethod == "repeat")
	{
		if (n_row_subsp%n)
		{
			ecl_error("ECL: ecl_datastruct::fill() - Repeat values do not match row range");
			return false;
		}

		long n_repeats = n_row_subsp/n;

		for (int i = 0; i < n_repeats; i++)
		{
			for(int j = 0; j < n; j++)
			{
				set_val( row_begin + (i*n_row_subsp) + j,var, argvals[j]);
			}
		}
	}

	
	// LINSPACE METHOD //

	else if(fillmethod == "linspace")
	{
		if (n != 3)
		{
			ecl_error("ECL: ecl_datastruct::fill() - Exactly three values needed for linspace method");
			return false;
		}

		long sign = 1;
		if (argvals[1] < 0)
		{sign = -1;}
		else if (argvals[1] == 0)
		{
			ecl_error("ECL: ecl_datastruct::fill() - Empty linspace");
			return false;
		}


		if (argvals[0]*sign>argvals[2])
		{
			ecl_error("ECL: ecl_datastruct::fill() - Empty linspace");
			return false;
		}

		if ((argvals[2]-argvals[0])%argvals[1])
		{
			ecl_error("ECL: ecl_datastruct::fill() - Invalid linspace");
			return false;
		}

		n = (argvals[2]-argvals[0])/argvals[1];
		
		if (n_row_subsp%n)
		{
			ecl_error("ECL: ecl_datastruct::fill() - Linspace repeats do not match row subspace");
			return false;
		}

		long n_repeats = n_row_subsp/n;

		for (int i = 0; i < n_repeats; i++)
		{
			for(int j = 0; j < n; j++)
			{
				set_val( row_begin + (i*n_row_subsp) + j, var, argvals[0] + (j*argvals[1]));
			}
		}
	}


	// RANDOM //

	else if(fillmethod == "random")
	{
		if (n != 2)
		{
			ecl_error("ECL: ecl_datastruct::fill() - Exactly two values needed for random method");
			return false;
		}

		long maxn = argvals[1]-argvals[0]+1;
		long rn; 
		
		for (int i = row_begin; i <= row_end; i++)
		{
			rn = rand () % maxn;
			set_val(i, var, argvals[0] + rn);
		}
	}


	// OTHER //
	else
	{
		ecl_error("ECL: ecl_datastruct::fill() - Unknown method");
		return false;
	}

	return true;
}

bool ecl_datastruct::shuffle(long row_begin, long row_end)
{
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::shuffle() - Data structure has not been built");
		return false;
	}
	
	if (row_begin < 1 || row_begin > m_n_rows || row_end < 1 || row_end > m_n_rows)
	{
		ecl_error("ECL: ecl_datastruct::shuffle() - Invalid row number");
		return false;
	}

	if (row_begin >= row_end)
	{
		ecl_error("ECL: ecl_datastruct::shuffle() - Start row number should be smaller than the end row number");
		return false;
	}

	trial t1;
	trial t2;
	long maxn = row_end-row_begin+1;
	long rn;

	for (long repeats = 0; repeats < 100; repeats++)
	{
		for (long i = row_begin; i <= row_end; i++)
		{
			rn = rand() % maxn;
			t1 = get_trial(i);
			t2 = get_trial(rn+row_begin);

			set_trial(rn+row_begin,t1);
			set_trial(i,t2);
		}
	}

	return true;
}

long ecl_datastruct::get_val(long row, long var)
{
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::get_val() - Data structure has not been built");
		return 0;
	}

	if(var < 0 || var >= m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::get_val() - Variable does not exist");
		return 0;
	}

	if (row < 1 || row > m_n_rows)
	{
		ecl_error("ECL: ecl_datastruct::get_val() - Row does not exist");
		return 0;
	}

	trial t = get_trial(row);
	return t.get_val(var);
}

void ecl_datastruct::set_val(long row, long var, long val)
{
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::set_val() - Data structure has not been built");
		return;
	}

	if(var < 0 || var >= m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::set_val() - Variable does not exist");
		return;
	}

	if (row < 1 || row > m_n_rows)
	{
		ecl_error("ECL: ecl_datastruct::set_val() - Row does not exist");
		return;
	}

	trial t = get_trial(row);
	t.set_val(var,val);
	set_trial(row,t);
	return;
}

ecl_datastruct::trial ecl_datastruct::get_trial(long row)
{
	ecl_datastruct::trial trial;
	
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::get_trial() - Data structure has not been built");
		return trial;
	}

	if (row < 1 || row > m_n_rows)
	{
		ecl_error("ECL: ecl_datastruct::get_trial() - Row does not exist");
		return trial;
	}

	return *m_data_matrix[row-1];
}

void ecl_datastruct::set_trial(long row, trial trial)
{
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::set_trial() - Data structure has not been built");
		return;
	}

	if (row < 1 || row > m_n_rows)
	{
		ecl_error("ECL: ecl_datastruct::set_trial() - Row does not exist");
		return;
	}

	if (m_n_vars != trial.m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::set_trial() - Incompatible number of variables");
		return;
	}

	for(int i = 0; i <= m_n_vars; i++)
	{
		m_data_matrix[row-1]->set_val(i,trial.m_trial_vector[i]);
	}

	return;
}

bool ecl_datastruct::load()
{
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::load - Data structure must first be built");
		return false;
	}
	
	if (m_fname == "")
	{
		ecl_error("ECL: ecl_datastruct::load - No filename specified");
		return false;
	}

	string fname = m_fname;
	fname.append(".trl");

	//FILE *f;
	//f->open(fname); // file must exist
	// Read one line
	// Determine whether the number of variables is correct
	// Parse it, save to datamatrix
	// Next line
	// Determine whether the number of rows is correct
	//f->close();

	return true;
}

bool ecl_datastruct::save(bool final_save)
{
	if(!m_b_built)
	{
		ecl_error("ECL: ecl_datastruct::save - Data structure must first be built");
		return false;
	}
	
	if (m_fname == "")
	{
		ecl_error("ECL: ecl_datastruct::save - No filename specified");
		return false;
	}

	string fname = m_fname;

	if(!final_save)
	{
		fname.append("_m%d.trl");
	//	sprintf(fname.c_str(),fname.c_str(),m_last_tmp_save);
		if(m_last_tmp_save){m_last_tmp_save++;}
		else{m_last_tmp_save--;}
	}
	else
	{fname.append(".trl");}

	//FILE *f;
	//f.open(fname); // file must exist
	// Write all lines
	//f.close();

	return true;
}

void ecl_datastruct::clear()
{
	trial * row;

	for (int r=0; r<m_n_rows; r++)
	{			
		row = m_data_matrix[r];
		row->clear();
		delete [] row;
		m_data_matrix[r] = NULL;
	}
	m_data_matrix.clear();
	
	m_b_built = false;
	m_n_rows = 0;
	m_n_vars = 0;
}



//***********************//
// ecl_datastruct::trial //
//***********************//                        

ecl_datastruct::trial::trial()
{
	m_n_vars = 0;
}

ecl_datastruct::trial::trial(string vals)
{
	m_n_vars = 0;
	init(vals);
}

ecl_datastruct::trial::~trial()
{
	clear();
}

void ecl_datastruct::trial::init(string vals)
{
	if (m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::trial::init - Trial is not empty");
		return;
	}
	
	istringstream iss(vals);
	do
	{		
		string sub;
		iss >> sub;
		m_trial_vector.push_back(strtol(sub.c_str(),NULL,1));
		m_n_vars++;
	} while (iss);

	if (m_n_vars < 2)
	{
		ecl_error("ECL: ecl_datastruct::trial::init - At least two variables needed");
		clear();
		return;
	}
}

void ecl_datastruct::trial::set_val(long var, long val)
{
	if(!m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::trial::set_val() - Trial is empty");
		return;
	}

	if(var < 0 || var >= m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::trial::set_val() - Variable does not exist");
		return;
	}

	m_trial_vector[var] = val;
}

long ecl_datastruct::trial::get_val(long var)
{
	if(!m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::trial::get_val() - Trial is empty");
		return 0;
	}

	if(var < 0 || var >= m_n_vars)
	{
		ecl_error("ECL: ecl_datastruct::trial::get_val() - Variable does not exist");
		return 0;
	}
	return m_trial_vector[var];
}

void ecl_datastruct::trial::clear()
{
	m_trial_vector.clear();
	m_n_vars = 0;
}