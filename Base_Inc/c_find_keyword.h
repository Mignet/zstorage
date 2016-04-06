#ifndef c_find_keyword_h
#define c_find_keyword_h

namespace find_keyword
{
	class c_keyword
	{
		typedef c_keyword*	KEYWORD[0XFF+1];
		typedef list<string> LWORDS;
	public:
		c_keyword():m_ending(false), m_depth(0), m_words_crc16(0) {memset(&m_keyword, 0, sizeof(KEYWORD));}
		~c_keyword() 
		{
			for (int i = 0; i < 0xFF + 1; i++)
			{
				if (m_keyword[i] != NULL)
					delete m_keyword[i];
			}
		}

		void result_words(uint16 crc, list<string>& words, uint16& crc16)
		{
			if (crc == m_words_crc16)
			{
				crc16 = 0;
				return;
			}

			for (LWORDS::iterator pos = m_words.begin(); pos != m_words.end(); ++pos)
			{
				words.push_back(*pos);
			}

			crc16 = m_words_crc16;
		}

		void push(const int8* keyword)
		{
			return push((uint8*)keyword);
		}

		void push(const uint8* keyword)
		{
			if (keyword == NULL) return;

			m_words.push_back((char*)keyword);
			m_words_crc16 = c_crc::crc16(m_words_crc16, (uint8*)keyword, strlen((char*)keyword));

			return p_keyword(keyword, this, 1);
		}

		int replace(int8* keyword, uint8 ch)
		{
			int result = 0;
			for (int i = 0; true; i++)
			{
				if (keyword[i] == '\0') break;

				int pos = key((const uint8*)&keyword[i]);
				if (pos > 0) 
				{
					int8* pkey = &keyword[i];
					for (int x = 0; x < pos; x++)
					{
						pkey[x] = ch;
					}

					i += pos - 1;
					result++;
				}
			}

			return result;
		}

		bool findkey(const int8* keyword, int& b, int& p)
		{
			for (int i = 0; true; i++)
			{
				if (keyword[i] == '\0') break;

				int pos = key((const uint8*)&keyword[i]);
				if (pos > 0) 
				{
					b = i;
					p = pos;
					return true;
				}
			}

			return false;
		}

		bool findkey(const int8* keyword)
		{
			for (int i = 0; true; i++)
			{
				if (keyword[i] == '\0') break;

				int pos = key((const uint8*)&keyword[i]);
				if (pos > 0) 
				{
					return true;
				}
			}

			return false;
		}

	private:
		

		int key(const uint8* keyword)
		{
			if (keyword == NULL) return 0;


			return f_keyword(keyword, this);
		}
		
		void p_keyword(const uint8* keyword, c_keyword* wkey, int depth)
		{
			if (keyword[0] == '\0') return;

			KEYWORD& key = wkey->m_keyword;

			if (key[keyword[0]] == NULL)
			{
				key[keyword[0]] = new c_keyword;
				key[keyword[0]]->m_depth = depth;
			}

			if (keyword[1] == '\0') key[keyword[0]]->m_ending = true;

			return p_keyword(keyword + 1, key[keyword[0]], ++depth);
		}

		int f_keyword(const uint8* keyword, c_keyword* wkey )
		{
			if (keyword[0] == '\0') return 0;

			KEYWORD& key = wkey->m_keyword;

			if (key[keyword[0]] == NULL)
				return 0;

			if (key[keyword[0]]->m_ending)
			{
				KEYWORD& next_key = key[keyword[0]]->m_keyword;
				if (next_key[keyword[1]] == NULL)
					return key[keyword[0]]->m_depth;
			}

			int pos = f_keyword(keyword + 1, key[keyword[0]]);

			if (pos == 0 && key[keyword[0]]->m_ending) 
				return key[keyword[0]]->m_depth;

			return pos;
		}

	protected:
		KEYWORD			m_keyword;		//树图
		bool			m_ending;		//是否关键字结尾(注: key1:中国  key2:中国人 会产生两个结点)
		int				m_depth;		//深度
		LWORDS			m_words;		//所有关键字
		uint16			m_words_crc16;	//crc16;
	};


}


#endif //c_find_keyword_h

