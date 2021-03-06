#ifndef __ENTROPYINFO_H__
#define __ENTROPYINFO_H__

#include "DSSTools.h"
#include "BitmapExt.h"

/* ------------------------------------------------------------------- */

class CEntropySquare
{
public :
	CPointExt					m_ptCenter;
	double						m_fRedEntropy;
	double						m_fGreenEntropy;
	double						m_fBlueEntropy;

private :
	void	CopyFrom(const CEntropySquare & es)
	{
		m_ptCenter		= es.m_ptCenter;
		m_fRedEntropy	= es.m_fRedEntropy;
		m_fGreenEntropy = es.m_fGreenEntropy;
		m_fBlueEntropy  = es.m_fBlueEntropy;
	};

public :
	CEntropySquare()
	{
	};

	CEntropySquare(const CPointExt & pt, double fRedEntropy, double fGreenEntropy, double fBlueEntropy)
	{
		m_ptCenter = pt;
		m_fRedEntropy	= fRedEntropy;
		m_fGreenEntropy = fGreenEntropy;
		m_fBlueEntropy	= fBlueEntropy;
	};

	CEntropySquare(const CEntropySquare & es)
	{
		CopyFrom(es);
	};

	virtual ~CEntropySquare()
	{
	};

	const CEntropySquare & operator = (const CEntropySquare & es)
	{
		CopyFrom(es);
		return (*this);
	};
};

typedef std::vector<CEntropySquare>	ENTROPYSQUAREVECTOR;

class CEntropyInfo
{
private :
	CSmartPtr<CMemoryBitmap>	m_pBitmap;
	LONG						m_lWindowSize;
	LONG						m_lNrPixels;
	LONG						m_lNrSquaresX,
								m_lNrSquaresY;
	std::vector<float>			m_vRedEntropies;
	std::vector<float>			m_vGreenEntropies;
	std::vector<float>			m_vBlueEntropies;
	CDSSProgress *				m_pProgress;

private :
	void	InitSquareEntropies();
	void	ComputeEntropies(LONG lMinX, LONG lMinY, LONG lMaxX, LONG lMaxY, double & fRedEntropy, double & fGreenEntropy, double & fBlueEntropy);
	void	GetSquareCenter(LONG lX, LONG lY, CPointExt & ptCenter)
	{
		ptCenter.X = lX * (m_lWindowSize * 2 + 1) + m_lWindowSize;
		ptCenter.Y = lY * (m_lWindowSize * 2 + 1) + m_lWindowSize;
	};

	void	AddSquare(ENTROPYSQUAREVECTOR & vSquares, LONG lX, LONG lY)
	{
		CEntropySquare			es;

		GetSquareCenter(lX, lY, es.m_ptCenter);
		es.m_fRedEntropy	= m_vRedEntropies[lX + lY * m_lNrSquaresX];
		es.m_fGreenEntropy	= m_vGreenEntropies[lX + lY * m_lNrSquaresX];
		es.m_fBlueEntropy	= m_vBlueEntropies[lX + lY * m_lNrSquaresX];

		vSquares.push_back(es);
	};

public :
	CEntropyInfo()
	{
		m_pProgress = NULL;
	};

	virtual ~CEntropyInfo()
	{
	};

	void	Init(CMemoryBitmap * pBitmap, LONG lWindowSize = 10, CDSSProgress * pProgress = NULL)
	{
		m_pBitmap.Attach(pBitmap);
		m_lWindowSize = lWindowSize;
		m_pProgress   = pProgress;
		InitSquareEntropies();
	};

	COLORREF16	GetPixel(LONG x, LONG y, double & fRedEntropy, double & fGreenEntropy, double & fBlueEntropy)
	{
		COLORREF16		crResult;
		LONG			lSquareX,
						lSquareY;

		crResult = m_pBitmap->GetPixel16(x, y);

		lSquareX = x / (m_lWindowSize * 2 + 1);
		lSquareY = y / (m_lWindowSize * 2 + 1);

		CPointExt			ptCenter;
		ENTROPYSQUAREVECTOR	vSquares;

		GetSquareCenter(lSquareX, lSquareY, ptCenter);
		AddSquare(vSquares, lSquareX, lSquareY);
		if (ptCenter.X > x)
		{
			if (lSquareX > 0)
				AddSquare(vSquares, lSquareX-1, lSquareY); 
		}
		else if (ptCenter.X < x)
		{
			if (lSquareX < m_lNrSquaresX - 1)
				AddSquare(vSquares, lSquareX+1, lSquareY);
		};

		if (ptCenter.Y > y)
		{
			if (lSquareY > 0)
				AddSquare(vSquares, lSquareX, lSquareY-1);
		}
		else if (ptCenter.Y < y)
		{
			if (lSquareY < m_lNrSquaresY - 1)
				AddSquare(vSquares, lSquareX, lSquareY+1);
		};

		// Compute the gradient entropy from the nearby squares
		fRedEntropy		= 0.0;
		fGreenEntropy	= 0.0;
		fBlueEntropy	= 0.0;
		CPointExt			ptPixel(x, y);
		double				fTotalWeight = 0.0;

		for (LONG i = 0;i<vSquares.size();i++)
		{
			double		fDistance;
			double		fWeight = 1.0;

			fDistance = Distance(ptPixel, vSquares[i].m_ptCenter);
			if (fDistance > 0)
				fWeight = 1.0/fDistance;

			fRedEntropy		+= fWeight * vSquares[i].m_fRedEntropy;
			fGreenEntropy	+= fWeight * vSquares[i].m_fGreenEntropy;
			fBlueEntropy	+= fWeight * vSquares[i].m_fBlueEntropy;

			fTotalWeight += fWeight;
		};

		fRedEntropy		/= fTotalWeight;
		fGreenEntropy	/= fTotalWeight;
		fBlueEntropy	/= fTotalWeight;

		return crResult;
	};
};

#endif // __ENTROPYINFO_H__