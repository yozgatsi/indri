/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// QueryEnvironment
//
// 9 March 2004 -- tds
//

#ifndef INDRI_QUERYENVIRONMENT_HPP
#define INDRI_QUERYENVIRONMENT_HPP

#include <map>
#include "indri/ScoredExtentResult.hpp"
#include "indri/QueryServer.hpp"
#include "indri/NetworkStream.hpp"
#include "indri/NetworkMessageStream.hpp"
#include "indri/Parameters.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/Repository.hpp"
#include "indri/QueryAnnotation.hpp"
#include "lemur/IndexTypes.hpp"
#include "indri/ReformulateQuery.hpp"

namespace indri
{
/*
  namespace lang
  {
    class QueryNodeWalker : public indri::lang::Walker {
    private:

    public:
      QueryNodeWalker() {}

      void defaultBefore( indri::lang::Node* n ) {

        std::cout << "indri::lang::QueryNodeWalker::defaultBefore()" << std::endl;
        std::cout << "\t n->nodeName() = " << n->nodeName() << std::endl;
        std::cout << "\t n->queryText() = " << n->queryText() << std::endl;
        std::cout << "\t n->typeName() = " << n->typeName() << std::endl;
      }

      void defaultAfter( indri::lang::Node* n ) {
      }

    };
  }
*/
  namespace api 
  {
    /*! \brief Structure for aggregating a query and its parameters.
      Provides an aggregate for specifying a query an retrieval parameters
      as a single argument. Can include parameters for snippet generation,
      metadata field retrieval, number of results, starting number of the
      results, and a working set of documents to evaluate the query on.
    */
    typedef struct QueryRequest
    {
      /*! Snippet generation options.
       */
      enum Options {
        /// Generate an html snippet with matches in &lt;strong&gt; tags.
        HTMLSnippet = 1,
        /// Generate a text snippet with matches in upper case.
        TextSnippet = 2
      };
      /// the query to run
      std::string query;
      /// the query formulators to employ (future)
      std::vector<std::string> formulators;
      /// the list of metadata fields to return
      std::vector<std::string>  metadata;
      /// the working set of documents to evaluate the query against, uses internal document ids.
      std::vector<lemur::api::DOCID_T>  docSet;
      /// number of results to return
      int resultsRequested;
      /// starting number in the result set, eg 10 to get results starting at the 11th position in the result list.
      int startNum;
      /// snippet generation options
      enum Options options;
    } QueryRequest;

    /*! encapsulation of a metadata field and its value
     */
    typedef struct MetadataPair
    {
      /// the metadata field name
      std::string key;
      /// the value of the field
      std::string value;
    } MetadataPair;

    /*! \brief Structure for aggregating a query result.
      Provides an aggregate for a query result based an retrieval parameters
      from a QueryRequest. Can include a text snippet, external document id,
      internal document id, metadata field values, score, extent begin, and
      extent end.
    */
    typedef struct QueryResult
    {
      /// text snippet generated by SnippetBuilder for this result
      std::string snippet;
      /// external document id
      std::string documentName;
      /// internal document id
      lemur::api::DOCID_T docid;
      /// query score
      double score;
      /// extent begin
      int begin;
      /// extent end
      int end;
      /// list of retrieved metadata fields
      std::vector<indri::api::MetadataPair> metadata;
    } QueryResult;

    /*! Aggretate of the list of QueryResult elements for a QueryRequest,
      with estimated number of total matches, query parse time, query
      query execution time, and parsed document processing time
      (metadata retrieval and snippet generation).
     */
    typedef struct QueryResults
    {
      /// time to parse the query in milliseconds
      float parseTime;
      /// time to evaluate the query in milliseconds
      float executeTime;
      /// time to retrieve metadata fields and generate snippets
      float documentsTime;
      /// estimated number of matches for the query
      int estimatedMatches;
      /// the list of QueryResult elements.
      std::vector<QueryResult> results;
    } QueryResults;

    /*! \brief Principal class for interacting with Indri indexes during retrieval.
      Provides the API for opening one or more Repository servers, either local
      or remote. Provides the API for querying the servers with the Indri
      query language, and additionally requesting aggregate collection
      statistics.
    */
    class QueryEnvironment {
    private:
      indri::api::Parameters reformulatorParams;
      indri::query::ReformulateQuery *reformulator;

      // first is entry in _servers, second is entry _streams
      // derive idx to erase from those.
      std::map<std::string, std::pair<indri::server::QueryServer *, indri::net::NetworkStream *> > _serverNameMap;
      std::vector<indri::server::QueryServer*> _servers;
      // first is entry _servers, second is entry in _repositories.
      // derive idx to erase from those.
      std::map<std::string, std::pair<indri::server::QueryServer *, indri::collection::Repository *> > _repositoryNameMap;
      std::vector<indri::collection::Repository*> _repositories;
      std::vector<indri::net::NetworkStream*> _streams;
      std::vector<indri::net::NetworkMessageStream*> _messageStreams;

      Parameters _parameters;
      bool _baseline;

      void _mergeQueryResults( indri::infnet::InferenceNetwork::MAllResults& results, std::vector<indri::server::QueryServerResponse*>& responses );
      void _copyStatistics( std::vector<indri::lang::RawScorerNode*>& scorerNodes, indri::infnet::InferenceNetwork::MAllResults& statisticsResults );

      std::vector<indri::server::QueryServerResponse*> _runServerQuery( std::vector<indri::lang::Node*>& roots, int resultsRequested );
      void _sumServerQuery( indri::infnet::InferenceNetwork::MAllResults& results, std::vector<indri::lang::Node*>& roots, int resultsRequested );
      void _mergeServerQuery( indri::infnet::InferenceNetwork::MAllResults& results, std::vector<indri::lang::Node*>& roots, int resultsRequested );
      void _annotateQuery( indri::infnet::InferenceNetwork::MAllResults& results, const std::vector<lemur::api::DOCID_T>& documentIDs, std::string& annotatorName, indri::lang::Node* queryRoot );

      std::vector<indri::api::ScoredExtentResult> _runQuery( indri::infnet::InferenceNetwork::MAllResults& results,
                                                             const std::string& q,
                                                             int resultsRequested,
                                                             const std::vector<lemur::api::DOCID_T>* documentIDs,
                                                             QueryAnnotation** annotation,
                                                             const std::string &queryType = "indri" );
      void _scoredQuery( indri::infnet::InferenceNetwork::MAllResults& results, indri::lang::Node* queryRoot, std::string& accumulatorName, int resultsRequested, const std::vector<lemur::api::DOCID_T>* documentSet );

      double _expCount( const std::string& expression,
                        const std::string& whichOccurrences,
                        const std::string &queryType = "indri" );

      QueryEnvironment( QueryEnvironment& other ) {}

    public:
      QueryEnvironment();
      ~QueryEnvironment();
      /// \brief Set the amount of memory to use.
      /// @param memory number of bytes to allocate
      void setMemory( UINT64 memory );
      /// \brief Set whether there should be one single background model or context sensitive models
      /// @param background true for one background model false for context sensitive models
      void setBaseline(const std::string &baseline);
      void setSingleBackgroundModel( bool background );
      /// \brief Set the scoring rules
      /// @param rules the vector of scoring rules.
      void setScoringRules( const std::vector<std::string>& rules );
      /// \brief Set the stopword list for query processing
      /// @param stopwords the list of stopwords
      void setStopwords( const std::vector<std::string>& stopwords );
      /// \brief Add a remote server
      /// @param hostname the host the server is running on
      void addServer( const std::string& hostname );
      /// \brief Add a local repository
      /// @param pathname the path to the repository.
      void addIndex( const std::string& pathname );
      /// Add an IndexEnvironment object.  Unlike the other add calls, this one
      /// will not close the index when QueryEnvironment::close is called.
      /// @param environment an IndexEnvironment instance
      void addIndex( class IndexEnvironment& environment );
      /// Close the QueryEnvironment.
      void close();
      /// \brief Remove a remote server
      /// @param hostname the host the server is running on
      void removeServer( const std::string& hostname );
      /// \brief Remove a local repository
      /// @param pathname the path to the repository.
      void removeIndex( const std::string& pathname );

      /// \brief Run an Indri query language query.
      /// @param request the query to run
      /// @return the QueryResults for the request,
      QueryResults runQuery(QueryRequest &request);

      /// \brief Run an Indri query language query. @see ScoredExtentResult
      /// @param query the query to run
      /// @param resultsRequested maximum number of results to return
      /// @return the vector of ScoredExtentResults for the query
      std::vector<indri::api::ScoredExtentResult> runQuery( const std::string& query, int resultsRequested, const std::string &queryType = "indri" );

      /// \brief Run an Indri query language query. @see ScoredExtentResult
      /// @param query the query to run
      /// @param documentSet the working set of document ids to evaluate
      /// @param resultsRequested maximum number of results to return
      /// @return the vector of ScoredExtentResults for the query
      std::vector<indri::api::ScoredExtentResult> runQuery( const std::string& query, const std::vector<lemur::api::DOCID_T>& documentSet, int resultsRequested, const std::string &queryType = "indri" );

      /// \brief Run an Indri query language query. @see QueryAnnotation
      /// @param query the query to run
      /// @param resultsRequested maximum number of results to return
      /// @return pointer to QueryAnnotations for the query
      QueryAnnotation* runAnnotatedQuery( const std::string& query, int resultsRequested, const std::string &queryType = "indri" );

      /// \brief Run an Indri query language query. @see QueryAnnotation
      /// @param query the query to run
      /// @param documentSet the working set of document ids to evaluate
      /// @param resultsRequested maximum number of results to return
      /// @return pointer to QueryAnnotations for the query
      QueryAnnotation* runAnnotatedQuery( const std::string& query, const std::vector<lemur::api::DOCID_T>& documentSet, int resultsRequested, const std::string &queryType = "indri" );


      /// \brief Fetch the parsed documents for a given list of document ids.
      /// Caller is responsible for deleting the returned elements.
      /// @param documentIDs the list of ids
      /// @return the vector of ParsedDocument pointers.
      std::vector<indri::api::ParsedDocument*> documents( const std::vector<lemur::api::DOCID_T>& documentIDs );
      /// \brief Fetch the parsed documents for a given list of ScoredExtentResults
      /// Caller is responsible for deleting the returned elements.
      /// @param results the list of ScoredExtentResults
      /// @return the vector of ParsedDocument pointers.
      std::vector<indri::api::ParsedDocument*> documents( const std::vector<indri::api::ScoredExtentResult>& results );
      /// \brief Fetch the named metadata attribute for a list of document ids
      /// @param documentIDs the list of ids
      /// @param attributeName the name of the metadata attribute
      /// @return the vector of string values for that attribute
      std::vector<std::string> documentMetadata( const std::vector<lemur::api::DOCID_T>& documentIDs, const std::string& attributeName );
      /// \brief Fetch the named metadata attribute for a list of ScoredExtentResults
      /// @param documentIDs the list of ScoredExtentResults
      /// @param attributeName the name of the metadata attribute
      /// @return the vector of string values for that attribute
      std::vector<std::string> documentMetadata( const std::vector<indri::api::ScoredExtentResult>& documentIDs, const std::string& attributeName );

      /// \brief Fetch the XPath names of extents for a list of ScoredExtentResults
      /// @param results the list of ScoredExtentResults
      /// @return the vector of string XPath names for the extents
      std::vector<std::string> pathNames( const std::vector<indri::api::ScoredExtentResult>& results );


      /// \brief Fetch all documents with a metadata key that matches attributeName, with a value matching one of the attributeValues.
      /// @param attributeName the name of the metadata attribute (e.g. 'url' or 'docno')
      /// @param attributeValues values that the metadata attribute should match
      /// @return a vector of ParsedDocuments that match the given metadata criteria
      std::vector<indri::api::ParsedDocument*> documentsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValues );

      /// \brief Return a list of document IDs where the document has a metadata key that matches attributeName, with a value matching one of the attributeValues.
      /// @param attributeName the name of the metadata attribute (e.g. 'url' or 'docno')
      /// @param attributeValue values that the metadata attribute should match
      /// @return a vector of ParsedDocuments that match the given metadata criteria
      std::vector<lemur::api::DOCID_T> documentIDsFromMetadata( const std::string& attributeName, const std::vector<std::string>& attributeValue );
      /// \brief Return the stem of the term
      /// @param term the term to stem
      /// @return the stem of the term
      std::string stemTerm(const std::string &term);
      /// \brief Return total number of unique terms.
      /// @return total number of unique terms in the aggregated collection
      INT64 termCountUnique();
      /// \brief Return total number of terms.
      /// @return total number of terms in the aggregated collection
      INT64 termCount();
      /// \brief Return total number of term occurrences.
      /// @param term the term to count
      /// @return total frequency of this term in the aggregated collection
      INT64 termCount( const std::string& term );
      /// \brief Return total number of stem occurrences.
      /// @param term the stem to count
      /// @return total frequency of this stem in the aggregated collection
      INT64 stemCount( const std::string& term );
      /// \brief Return total number of term occurrences within a field.
      /// @param term the term to count
      /// @param field the name of the field
      /// @return total frequency of this term within this field in the
      /// aggregated collection
      INT64 termFieldCount( const std::string& term, const std::string& field );
      /// \brief Return total number of stem occurrences within a field.
      /// @param term the stem to count
      /// @param field the name of the field
      /// @return total frequency of this stem within this field in the
      /// aggregated collection
      INT64 stemFieldCount( const std::string& term, const std::string& field );
      /// \brief Return the total number of times this expression appears in the collection.
      /// @param expression The expression to evaluate, probably an ordered or unordered window expression
      double expressionCount( const std::string& expression,
                              const std::string &queryType = "indri" );
      /// \brief Return the total number of documents this expression appears in the collection.
      /// @param expression The expression to evaluate, probably an ordered or unordered window expression
      double documentExpressionCount( const std::string& expression,
                              const std::string &queryType = "indri" );
      /// \brief Return all the occurrences of this expression in the collection.
      /// Note that the returned vector may be quite large for large collections, and therefore
      /// has the very real possibility of exhausting the memory of the machine.  Use this method
      /// with discretion.
      /// @param expression The expression to evaluate, probably an ordered or unordered window expression
      std::vector<ScoredExtentResult> expressionList( const std::string& expression,
                                                      const std::string& queryType = "indri" );
      /// \brief Return the list of fields.
      /// @return vector of field names.
      std::vector<std::string> fieldList();
      /// \brief Return total number of documents in the collection.
      /// @return total number of documents in the aggregated collection
      INT64 documentCount();
      /// \brief Return total number of documents containing term in the collection.
      /// @param term the term to count documents for.
      /// @return total number of documents containing term in the aggregated collection
      INT64 documentCount( const std::string& term );

      /// \brief Return total number of documents containing stem in the collection.
      /// @param stem the prestemmed term to count documents for.
      /// @return total number of documents containing stem in the aggregated collection
      INT64 documentStemCount( const std::string& stem );

      /// \brief Return the length of a document.
      /// @param documentID the document id.
      /// @return length of the document, documentID
      int documentLength(lemur::api::DOCID_T documentID);

      /// \brief Fetch a document vector for a list of documents.
      /// Caller responsible for deleting the Vector.
      /// @param documentIDs the vector of document ids.
      /// @return DocumentVector pointer for the specified document.
      std::vector<DocumentVector*> documentVectors( const std::vector<lemur::api::DOCID_T>& documentIDs );

      /// \brief set maximum number of wildcard terms to expand to.
      /// @param maxTerms the maximum number of terms to expand a wildcard
      /// operator argument (default 100).
      void setMaxWildcardTerms(int maxTerms);

      /// \brief return the internal query servers.
      /// @return the local and network query servers.
      const std::vector<indri::server::QueryServer*>& getServers() const {
        return _servers;
      }

      /// \brief set the query reformulation parameters.
      /// @param p the Parameters object containing the parameters.
      void setFormulationParameters(Parameters &p);

      /// \brief reformulate a query.
      /// @param query the bag of words query to reformulate.
      /// @return the indri query language reformulated query.
      std::string reformulateQuery(const std::string &query);
    };
  }
}

#endif // INDRI_QUERYENVIRONMENT_HPP
