

//
// SnippetBuilder
//
// This code is based largely on the code I wrote for the PHP and Java
// interfaces.
//
// 17 July 2006 -- tds
//

#include "indri/SnippetBuilder.hpp"
#include <algorithm>
#include <set>

//
// _getRawNodes
//

void indri::api::SnippetBuilder::_getRawNodes( std::vector<std::string>& nodeNames, const indri::api::QueryAnnotationNode* node ) {

  if( node->type == "RawScorerNode" ) {

    /*
     *
    std::cout << "indri::api::SnippetBuilder::_getRawNodes" << std::endl;
    std::cout << "\t node->name = " << node->name;
    std::cout << ", node->type = " << node->type;
    std::cout << ", node->queryText = " << node->queryText << std::endl;
     *
     */

    nodeNames.push_back( node->name );
  } else {

    //std::cout << "\t node->children.size() = " << node->children.size() << std::endl;

    for( size_t i=0; i<node->children.size(); i++ ) {
      _getRawNodes( nodeNames, node->children[i] );
    }
  }
}

//
// sort_extent_pairs
//

bool sort_extent_pairs( const std::pair< indri::index::Extent, int >& one,
                        const std::pair< indri::index::Extent, int >& two ) {
  return one.first.beginsBefore( two.first );
}

//
// sort_regions
//

bool sort_regions( const indri::api::SnippetBuilder::Region& one,
                   const indri::api::SnippetBuilder::Region& two )
{
  return one.begin < two.begin;
}

//
// _documentMatches
//

std::vector< std::pair<indri::index::Extent, int> > indri::api::SnippetBuilder::_documentMatches( int document,
                                                            const std::map< std::string,
                                                            std::vector<indri::api::ScoredExtentResult> >& annotations,
                                                            const std::vector<std::string>& nodeNames ) {
  std::vector< std::pair<indri::index::Extent, int> > extents;

  //std::cout << "indri::api::SnippetBuilder::_documentMatches" << std::endl;
  //std::cout << "\t nodeNames.size() = " << nodeNames.size() << std::endl;

  for( size_t i=0; i<nodeNames.size(); i++ ) {
    std::map< std::string, std::vector<indri::api::ScoredExtentResult> >::const_iterator iter;
    const std::string& nodeName = nodeNames[i];

    //std::cout << "\t\t nodeNames[" << i << "] = " << nodeName;

    // are there annotations for this node?  if not, keep going
    iter = annotations.find( nodeName );
    if( iter == annotations.end() ) {

      //std::cout << ", no annotations for this node." << std::endl;

      continue;
    }

    // there are annotations, so get just the ones for this document
    const std::vector<indri::api::ScoredExtentResult>& matches = iter->second;

    //std::cout << ", matches.size() = " << matches.size() << std::endl;

    for( size_t j=0; j<matches.size(); j++ ) {

      //std::cout << "\t\t\t matches[" << j << "].document = " << matches[j].document;
      //std::cout << ", matches[" << j << "].begin = " << matches[j].begin;
      //std::cout << ", matches[" << j << "].end = " << matches[j].end << std::endl;

      if( matches[j].document == document ) {
        indri::index::Extent e;
        e.begin = matches[j].begin;
        e.end = matches[j].end;

        extents.push_back( std::make_pair( e, i ) );
      }
    }
  }

  // now we have some extents; we'll sort them by start position
  std::sort( extents.begin(), extents.end(), sort_extent_pairs );
  return extents;
}

bool should_skip( const std::vector< indri::api::SnippetBuilder::Region >& skips, int begin, int end ) {
  for( size_t i=0; i<skips.size(); i++ ) {
    if( skips[i].begin <= begin && skips[i].end >= end )
      return true;
  }

  return false;
}

//
// _bestRegion
//

indri::api::SnippetBuilder::Region indri::api::SnippetBuilder::_bestRegion(
  const std::vector< std::pair<indri::index::Extent, int> >& extents,
  const std::vector< indri::api::SnippetBuilder::Region >& skipRegions,
  int positionCount, int windowWidth ) {
  // try to find as many unique occurrences as possible
  Region best;
  size_t bestUnique = 0;
  std::set<int> bestSet;
  best.begin = 0;
  best.end = 0;

  std::vector< Region >::const_iterator skipIter = skipRegions.begin();

  for( size_t i=0; i<extents.size(); i++ ) {
    if( should_skip( skipRegions, extents[i].first.begin, extents[i].first.end ) )
      continue;

    // if this extent is past the end, it doesn't count
    if( extents[i].first.begin >= positionCount )
      break;

    // okay, now let's really look for a nice extent
    Region region;
    region.begin = extents[i].first.begin;
    region.end = extents[i].first.end;
    region.extents.push_back( extents[i].first );

    std::set<int> nodes;
    nodes.insert( extents[i].second );
    size_t j;

    for( j=i; j<extents.size(); j++ ) {
      int newEnd = std::max( extents[j].first.end, region.end );

      if( newEnd - region.begin > windowWidth || should_skip( skipRegions, extents[j].first.begin, extents[j].first.end ) )
        break;

      // remove duplicate and/or overlapping extents
      if( region.extents.back().end < extents[j].first.begin ) {
        region.extents.push_back( extents[j].first );
      } else {
        region.extents.back().end = extents[j].first.end;
      }

      nodes.insert( extents[j].second );
      region.end = newEnd;
    }

    if( bestUnique < nodes.size() ) {
      best = region;
      bestUnique = nodes.size();
      bestSet = nodes;
    }
  }

  return best;
}

//
// _buildRegions
//


std::vector<indri::api::SnippetBuilder::Region> indri::api::SnippetBuilder::_buildRegions(
  std::vector< std::pair<indri::index::Extent, int> >& extents,
  int positionCount, int matchWidth, int windowWidth )
{
  std::vector<indri::api::SnippetBuilder::Region> regions;
  int wordCount = 0;

  if( extents.size() == 0 )
    return regions;

  std::vector<Region> matchRegions;
  int wordsUsed = 0;

  // find the best possible extents (in terms of coverage) that we possibly can
  // bias toward the document beginning
  while( wordsUsed < windowWidth ) {
    Region matchRegion = _bestRegion( extents, matchRegions, positionCount, windowWidth - wordsUsed );
    wordsUsed += matchRegion.end - matchRegion.begin;

    if( matchRegion.end - matchRegion.begin == 0 )
      break;

    matchRegions.push_back( matchRegion );
    std::sort( matchRegions.begin(), matchRegions.end(), sort_regions );
  }

  // now we have some match regions, so put together some reasonable context for them
  // BUGBUG: need additional logic here to ensure we don't get overlap between the regions.
  for( size_t i=0; i<matchRegions.size(); i++ ) {
    matchRegions[i].begin = std::max( 0, matchRegions[i].begin - matchWidth / 2 );
    matchRegions[i].end = std::min( positionCount, matchRegions[i].end + matchWidth / 2 );
  }

  return matchRegions;
}

//
// _sanitizeText
//

char* indri::api::SnippetBuilder::_sanitizeText( const char* text, int begin, int length ) {
  char* result = (char*) malloc( length+1 );
  memset( result, 0, length+1 );

  bool wasSpace = false;
  int end = begin+length;
  int j=0;

  for( int i=begin; i<end; i++ ) {
    char c = text[i];

    if( c == '<' ) {
      i++;

      // skip past any whitespace
      i += strspn( text + i, " \t\n\r" );

      if( !::strncmp( "!--", text + i, 3 ) ) {
        // in comment, search for end of it:
        i += 3;
        const char* endp = strstr( "-->", text + i );
        if( endp )
          i = endp - text + 2;
        else
          i = end;
      } else if( i < length-5 && !lemur_compat::strncasecmp( "style", text + i, 5 ) ) {
        // style tag
        const char* endp = lemur_compat::strcasestr( "</style", text + i );
        if( endp )
          endp = strchr( endp, '>' );
        if( endp )
          i = endp - text;
        else
          i = end;
      } else if( i < length-6 && !lemur_compat::strncasecmp( "script", text + i, 6 ) ) {
        // script tag
        const char* endp = lemur_compat::strcasestr( "</script", text + i );
        if( endp )
          endp = strchr( endp, '>' );
        if( endp )
          i = endp - text;
        else
          i = end;
      } else {
        // regular old everyday tag, skip it
        const char* endp = strchr( text + i, '>' );
        if( endp )
          i = endp - text;
        else
          i = end;
      }
    } else if( isspace(c) ) {
      // allow one space between words
      if( !wasSpace ) {
        wasSpace = true;
        result[j++] = ' ';
      }
    } else {
      result[j++] = c;
      wasSpace = false;
    }
  }

  result[j] = 0;
  return result;
}

//
// _addHighlightedText
//

void indri::api::SnippetBuilder::_addHighlightedRegion( std::string& snippet, char* region ) {
  if( _HTMLOutput ) {
    snippet += "<strong>";
    snippet += region;
    snippet += "</strong>";
  } else {
    for( char* c = region; *c; c++ ) {
      *c = toupper(*c);
    }
    snippet += region;
  }
  free(region);
}

//
// _addUnhighlightedText
//

void indri::api::SnippetBuilder::_addUnhighlightedRegion( std::string& snippet, char* region ) {
  if( _HTMLOutput )
    snippet += region;
  else
    snippet += region;

  free(region);
}

//
// _addEllipsis
//

void indri::api::SnippetBuilder::_addEllipsis( std::string& snippet ) {
  if( _HTMLOutput )
    snippet += "<strong>...</strong>";
  else
    snippet += "...";
}

//
// _completeSnippet
//

void indri::api::SnippetBuilder::_completeSnippet( std::string& snippet ) {
  if( _HTMLOutput )
    return;

  size_t i = 0;

  // add linebreaks
  while( i < snippet.size() ) {
    i += 50;
    while( i < snippet.size() && !isspace(snippet[i]) )
      i++;
    if( i < snippet.size() )
      snippet[i] = '\n';
  }
}

//
// build
//

std::string indri::api::SnippetBuilder::build( int documentID, const indri::api::ParsedDocument* document, indri::api::QueryAnnotation* annotation ) {
  // put together the match information we'll need later
  int windowSize = 50;
  const char* text = document->text;
  std::vector<std::string> nodeNames;
  _getRawNodes( nodeNames, annotation->getQueryTree() );
  std::vector< std::pair<indri::index::Extent, int> > extents = _documentMatches( documentID, annotation->getAnnotations(), nodeNames );

  /*
   *
  std::cout << "indri::api::SnippetBuilder::build" << std::endl;
  std::cout << "\t document->positions.size() = " << document->positions.size() << std::endl;
  std::string docText;
  int bb;
  int eb;
  int len;
  for(size_t i=0; i<document->positions.size(); i++ ) {
    bb = document->positions[i].begin;
    eb = document->positions[i].end;
    len = eb - bb;
    docText += _sanitizeText( text, bb, len );
    std::cout << "\t\t docText = " << docText << std::endl;
  }
  std::cout << "\t nodeNames.size() = " << nodeNames.size() << std::endl;
  std::cout << "\t extents.size() = " << extents.size() << std::endl;
  for(size_t i=0; i<nodeNames.size(); i++ ) {
    std::cout << "\t\t nodeNames[ " << i << " ] = " << nodeNames[i] << std::endl;
  }
  for(size_t i=0; i<extents.size(); i++ ) {
    std::cout << "\t\t extents[ " << i << " ].first.begin = " << extents[i].first.begin;
    std::cout << ", extents[ " << i << " ].first.end = " << extents[i].first.end;
    std::cout << ", extents[ " << i << " ].second = " << extents[i].second << std::endl;
  }
   *
   */

  if( extents.size() == 0 )
    return std::string();

  // calculate the context width for a single match
  int matchWidth = windowSize /(int) extents.size();
  matchWidth = std::max( 15, std::min<int>( 30, extents.size() ) );

  // first, we compute a list of regions
  std::vector<indri::api::SnippetBuilder::Region> regions;
  regions = _buildRegions( extents, document->positions.size(), matchWidth, windowSize );

  /*
   *
  std::cout << "\t regions.size() = " << regions.size() << std::endl;
  std::cout << "\t windowSize = " << windowSize << std::endl;
  std::cout << "\t matchWidth = " << matchWidth << std::endl;
   */

  // now, we have to put all of these regions together into a snippet
  std::string snippet;
  int wordCount = 0;

  for( size_t i=0; i<regions.size() && windowSize > wordCount; i++ ) {
    Region& region = regions[i];

    if( region.begin != 0 && i == 0 ) {
      _addEllipsis( snippet );
    }

	  if( region.end > (int)document->positions.size() )
	    continue;

    int beginByte = document->positions[region.begin].begin;
    int endByte = document->positions[region.end-1].end;
    int current = beginByte;
    wordCount += region.end - region.begin;


    /*
     *
    std::cout << "\t region = " << i << std::endl;
    std::cout << "\t\t region.begin = " << region.begin;
    std::cout << ", region.end = " << region.end << std::endl;
    std::cout << "\t\t beginByte = " << beginByte;
    std::cout << ", endByte = " << endByte;
    std::cout << ", wordCount = " << wordCount << std::endl;
    std::cout << "\t\t region.extents.size() = " << region.extents.size() << std::endl;
     */


    for( size_t j=0; j<region.extents.size(); j++ ) {
      int regionBegin = region.extents[j].begin;
      int regionEnd = region.extents[j].end;

	  if( regionEnd > (int)document->positions.size() )
		continue;

      int beginMatch = document->positions[regionBegin].begin;
      int endMatch = document->positions[regionEnd-1].end;


      /*
       *
      std::cout << "\t\t\t region extent = " << j << std::endl;
      std::cout << "\t\t\t\t regionBegin = " << regionBegin;
      std::cout << ", regionEnd = " << regionEnd << std::endl;
      std::cout << "\t\t\t\t beginMatch = " << beginMatch;
      std::cout << ", endMatch = " << endMatch << std::endl;
       */


      _addUnhighlightedRegion( snippet, _sanitizeText( text, current, beginMatch - current ) );
      _addHighlightedRegion( snippet, _sanitizeText( text, beginMatch, endMatch - beginMatch ) );

      current = endMatch;
    }

    _addUnhighlightedRegion( snippet, _sanitizeText( text, current, endByte - current ) );

    if( region.end != document->positions.size()-1 )
      _addEllipsis( snippet );
  }

  _completeSnippet( snippet );
  return snippet;
}

//
// buildField
//

std::vector< std::pair< std::string, std::string > >* indri::api::SnippetBuilder::buildField( int documentID, const indri::api::ParsedDocument* document, indri::api::QueryAnnotation* annotation, std::vector< std::string > fields ) {
  // put together the match information we'll need later
  int windowSize = 50;
  const char* text = document->text;
  std::vector<std::string> nodeNames;
  _getRawNodes( nodeNames, annotation->getQueryTree() );
  std::vector< std::pair<indri::index::Extent, int> > extents = _documentMatches( documentID, annotation->getAnnotations(), nodeNames );

  int beginByte;
  int endByte;
  size_t len;
  std::string snippet;
  std::string str;
  std::stringstream ss;

  for(size_t i=0; i<extents.size(); i++ ) {

    beginByte = document->positions[extents[i].first.begin].begin;
    endByte = document->positions[extents[i].first.end-1].end;
    len = endByte - beginByte;


    char* s = (char*) malloc( len+1 );
    memset( s, 0, len+1 );
    strncpy( s, &text[beginByte], len );

    //str.resize( len );
    //str.clear();
    //strncpy( str.c_str(), ch, len );

    ss << "   extent = " << i << ", nodeName = " << nodeNames[extents[i].second] << ", text = " << s << std::endl;
    free( s );
  }
  snippet = ss.str();

  std::cout << "indri::api::SnippetBuilder::buildField" << std::endl;
  std::cout << " document = " << documentID << std::endl;
  std::cout << snippet;
  /*
   *
   *
   */




  int bb;
  int eb;
  //int len;
  int docLen;
  std::vector< std::pair< std::string, std::string > >* fds = new std::vector< std::pair< std::string, std::string > >;

  /*
   *
  std::cout << "indri::api::SnippetBuilder::buildField" << std::endl;
  std::cout << "\t document->terms.size() = " << document->terms.size() << std::endl;
  std::cout << "\t document->tags.size() = " << document->tags.size() << std::endl;
  std::cout << "\t document->metadata.size() = " << document->metadata.size() << std::endl;
  std::cout << "\t document->positions.size() = " << document->positions.size() << std::endl;
  //
  // text and content seem to be the same for html fce
  //std::cout << "\t document->text = " << std::string( document->text, document->textLength ) << std::endl;
  std::cout << "\t document->content = " << std::string( document->content, document->contentLength )  << std::endl;

  std::string docText;
  std::string unsText;
  docLen = 0;
  for(size_t i=0; i<document->positions.size(); i++ ) {
    bb = document->positions[i].begin;  char* s = (char*) malloc( docLen+1 );
  memset( s, 0, docLen+1 );
  bb = document->positions[0].begin;
  strncpy( s, &text[bb], docLen );

    eb = document->positions[i].end;
    len = eb - bb;
    docLen += len;
    docText += _sanitizeText( text, bb, len );
  }
  unsText.resize( docLen );
  unsText.clear();

  char* s = (char*) malloc( docLen+1 );
  memset( s, 0, docLen+1 );
  bb = document->positions[0].begin;
  strncpy( s, &text[bb], docLen );

  unsText = s;
   *
   */

  /*
   *
  std::cout << "\t\t docText = " << docText << std::endl;
  std::cout << "\t\t unsText = " << unsText << std::endl;

  std::string k = fields[0];
  std::string v = unsText;
  (*fds).push_back( std::make_pair( k, v ) );
   *
   */

  return fds;

}
