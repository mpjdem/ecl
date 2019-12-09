#include <string>
#include <vector>

#include "ecl.h"

using namespace std;

enum vars {vTRNR, vCOND, vRT, vEVAL, vN};


class ecl_datastruct
{
public:
										ecl_datastruct();
	virtual							   ~ecl_datastruct();


	class trial
	{
	public:
										trial();
										trial(string vals);
	virtual								~trial();
			
				void					init(string vals);
				void					set_val(long var, long val);
				long					get_val(long var);

				void					clear();

	protected:
	friend class ecl_datastruct;
				vector<long>			m_trial_vector;
				long					m_n_vars;
	};


	inline	void						set_fname(string fname){m_fname=fname;}
	inline	bool						is_valid(){return m_b_built;} 

			bool						build(long n_rows, enum vars);
			bool						fill(long row_begin, long row_end, long var, string fillmethod, string vals);
			bool						shuffle(long row_begin, long row_end);

			long						get_val(long row, long var);
			void						set_val(long row, long var, long val);
			
			trial						get_trial(long row);
			void						set_trial(long row, trial trial);

			bool						load();
			bool						save(bool final_save=false);

			void						clear();


protected:
		vector<trial*>					m_data_matrix;

		bool							m_b_built;
			
		string							m_fname;
		long							m_n_rows;
		long							m_n_vars;

		long 							m_last_tmp_save;
};