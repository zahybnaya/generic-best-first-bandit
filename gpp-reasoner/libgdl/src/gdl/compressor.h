/*
 * compressor.h
 *
 *  Created on: 29.03.2009
 *      Author: nos
 */

#ifndef COMPRESSOR_H_
#define COMPRESSOR_H_

#include <map>
#include "knowledgebase.h"
#include "kif.h"
/**
 * Compressor compresses a knowledge base and generates compression/deompression functions
 * 
 * Compression works this way:
 * - not/distinct/or/and are not compressed
 * - all other are beginning from their first letter and then going from a..z, 0..9 (multiple letters
 *   if there are not enough)
 * 
 * Note that you have to bring all commands through the compressor when working on a compressed KnowledgeBase
 * also you have to decompress all results before putting them on the web
 */
class Compressor {
public:
	Compressor();
	/// generates a compressed KnowledgeBase out of a KIF instance
	void init (const KIF & kif, KnowledgeBase & target);
	
	bool compress (const std::string & src, std::string & target) const;
	
	bool decompress (const std::string & src, std::string & target) const;
	
	virtual ~Compressor();
private:
	/// initial compresses a string; filling compMap / decompMap
	std::string initialCompress (const std::string&);
	
	std::string genCompressed (const std::string & x);
	bool reserved (const std::string & x) const { return m_decompressMap.find(x) != m_decompressMap.end(); }
	
	
	typedef std::map<std::string, std::string> CompMap;
	CompMap m_compressMap;
	CompMap m_decompressMap;
	
	std::string m_currToken; 		// current regular token to use
	std::string m_currVariable;		// current variable to use
	
};

#endif /* COMPRESSOR_H_ */
