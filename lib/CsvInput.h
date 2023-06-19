#ifndef	__CSV_INPUT_H__
#define	__CSV_INPUT_H__

#define	MIN_DATA_SIZE	 4096
#define	MIN_ITEM_NUMS	 1024

class TCsvInput
{
public:
	TCsvInput();
	TCsvInput(int iMaxDataSize, int iMaxItemNums);
	~TCsvInput();

	int		Open(char *szFileName);
	void		Close();

	int		ReadAsHead();
	int		GetHeadNums();
	char	*	GetHeadValu(int iIndex);
	int		GetIndex(char *szHeadName);

	int		Read();
	int		ReadFromBuffer(char *szBuffer);
	int		GetItemNums();
	char	*	GetItemValu(int iIndex);

	void		DisplayHead();
	void		DisplayItem();

protected:
private:
	void		Init(int iMaxDataSize, int iMaxItemNums);
	void		DeFormat(char *szValue);
	int		Parse(char *szBuff, char **szValu);

	//
	FILE	*	m_pCsvFd;

	//
	int		m_iMaxDataSize;
	int		m_iMaxItemNums;

	//
	int		m_iHeadNums;
	char	*	m_pHeadBuff;
	char	**	m_pHeadValu;

	//
	int		m_iItemNums;
	char	*	m_pItemBuff;
	char	**	m_pItemValu;
};

#endif
