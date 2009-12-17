/*
 * Copyright (C) 2008-2009 Advanced Micro Devices, Inc.  All Rights Reserved.
 */

//-*-c++-*-

/*
 * Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

// ====================================================================
// ====================================================================
//
// Module: opt_cfg_trans.h
// $Revision: 1.1.1.1 $
// $Date: 2005/10/21 19:00:00 $
// $Author: marcel $
// $Source: /proj/osprey/CVS/open64/osprey1.0/be/opt/opt_cfg_trans.h,v $
//
// ====================================================================
//
// Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it would be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Further, this software is distributed without any warranty that it
// is free of the rightful claim of any third person regarding
// infringement  or the like.  Any license provided herein, whether
// implied or otherwise, applies only to this software file.  Patent
// licenses, if any, provided herein do not apply to combinations of
// this program with other software, or any other product whatsoever.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
//
// Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
// Mountain View, CA 94043, or:
//
// http://www.sgi.com
//
// For further information regarding this notice, see:
//
// http://oss.sgi.com/projects/GenInfo/NoticeExplan
//
// ====================================================================
//
// Description:
//
//   -- implements generic graph library (cluster_vector)
//   -- instantiate predecessor_graph and successor_graph for
//       control flow transformation
//   -- defines the "zone" data structure
//   -- defines the "path" data structure
//
// ====================================================================
// ====================================================================


#ifndef opt_cfg_trans_INCLUDED
#define opt_cfg_trans_INCLUDED "opt_cfg_trans.h"

#include <iterator>
#include <set>
#include <map>
#include <functional>

#define USE_STANDARD_TYPE
#include "opt_defs.h"   // use Is_True
#include <stdio.h>
#include "opt_bb.h"
#include "opt_base.h"

extern "C" {
#include "bitset.h"
}

typedef int vertex_id;

template <class Cluster_iterator>
bool adjacent(Cluster_iterator c1, const Cluster_iterator c2) {
  // assert(c1 < c2)
  ++c1;
  while (c1 < c2) {
    if (c1->begin() != c1->end()) return false;
    ++c1;
  }
  return true;
}

// composite_iterator -- 
//    an generic iterator build from the cluster_iterator and
//    fast_iterator of cluster (hierarchical) containers
//
template <class Cluster_iterator, class Fast_iterator>
class composite_iterator {
public:
  typedef composite_iterator<Cluster_iterator, Fast_iterator> self;
  typedef forward_iterator_tag iterator_category;
  typedef typename std::iterator_traits<Fast_iterator>::value_type      value_type;
  typedef typename std::iterator_traits<Fast_iterator>::difference_type 
          difference_type;
  typedef typename std::iterator_traits<Fast_iterator>::pointer         pointer;
  typedef typename std::iterator_traits<Fast_iterator>::reference       reference;

  Cluster_iterator ci;
  Fast_iterator fi;

  inline void normalize_forward() {
    while (fi == (*ci).end()) {
      ++ci;
      fi = (*ci).begin();
    }
  }
  inline void normalize_backward() {
    while (fi == (*ci).begin()) {
      --ci;
      fi = (*ci).end();
    }
  }

public:
  value_type& operator*() {
    normalize_forward();
    return *fi;
  }
  self& operator++() {
    if (fi == (*ci).end())
      normalize_forward();
    ++fi;
    return *this;
  }
  self& operator--() {
    if (fi == (*ci).begin())
      normalize_backward();
    --fi;
    return *this;
  }
  self operator++(int) {
    self tmp = *this;
    if (fi == (*ci).end())
      normalize_forward();
    ++fi;
    return tmp;
  }
  self operator--(int) {
    self tmp = *this;
    if (fi == (*ci).begin())
      normalize_backward();
    --fi;
    return tmp;
  }
  self& operator+=(int distance) {
    while (distance > 0) {
      normalize_forward();
      int delta = min((*ci).end()-fi, distance);
      // normalize_forward() ==> fi != (*ci).end() ==> delta > 0
      fi += delta;
      distance -= delta;
    }
    return *this;
  }
  self operator-=(int distance) {
    while (distance > 0) {
      int delta = min(fi-(*ci).begin(), distance);
      fi -= delta;
      distance -= delta;
      if (distance > 0) {
	// distance > 0 ==> fi == (*ci.begin())
	normalize_backward();
	fi = (*ci).end();
	--fi;
	--distance;
      }
    }
  }
  friend bool adjacent(Cluster_iterator c1, const Cluster_iterator c2) {
    // assert(c1 < c2)
    ++c1;
    while (c1 < c2) {
      if (c1->begin() != c1->end()) return false;
      ++c1;
    }
    return true;
  }
  friend bool operator==(const self& x, const self& y) {
    if (x.ci == y.ci)
      return x.fi == y.fi;
    if (x.fi == (*x.ci).end())
      return y.fi == (*y.ci).begin() && adjacent(x.ci,y.ci);
    if (x.fi == (*x.ci).begin())
      return y.fi == (*y.ci).end() && adjacent(y.ci,x.ci);
    return false;
  }

  Cluster_iterator cluster_iterator() const { return ci; }
  Fast_iterator fast_iterator() const { return fi; }

  composite_iterator<Cluster_iterator, Fast_iterator>
  (Cluster_iterator t1, Fast_iterator t2):ci(t1),fi(t2){}
};


//  cluster_vector is a vector of vector of data.
//  An index function is provided to select from the first vector.
//
template <class T, class IndexFunction>
class cluster_vector {
public:
  typedef typename IndexFunction::result_type cluster_id;
  typedef T value_type;
  typedef IndexFunction index_function;
  typedef cluster_vector<T, IndexFunction> self;
  typedef vector<value_type> cluster_type;
  typedef typename cluster_type::iterator fast_iterator;
  typedef typename cluster_type::size_type size_type;
  typedef vector<cluster_type> cluster_container;
  typedef typename cluster_container::iterator cluster_iterator;
  typedef composite_iterator<cluster_iterator, fast_iterator> iterator;

private:
  cluster_container cluster;
  index_function    i_fun;

public:
  cluster_iterator cluster_begin()   { return cluster.begin(); }
  cluster_iterator cluster_end()     { return cluster.end(); }
  size_type size() const                  { return cluster.size(); }
  cluster_type& operator[](cluster_id id) { 
    Is_True(id < cluster.size(), ("cluster_vector out-of-bound access."));
    return cluster[id]; 
  }
  cluster_id cluster_index(cluster_iterator ci) { return ci - cluster_begin(); }

  void extend(cluster_id id) {
    if (id >= cluster.size()) 
      cluster.insert(cluster.end(), id - cluster.size() + 1, cluster_type());
  }
  iterator insert(const value_type& v) {
    cluster_id id = i_fun(v);
    if (id < cluster.size()) {
      cluster[id].push_back(v);
      return iterator(cluster.begin()+id, cluster[id].end()-1);
    }
    // extend container and insert at the end
    extend(id);
    cluster[id].push_back(v);
    return iterator(cluster.begin()+id, cluster[id].end()-1);
  }

  iterator begin()   { return iterator(cluster_begin(), (*cluster_begin()).begin()); }
  iterator end()     { return iterator(cluster_end()-1, (*(cluster_end()-1)).end()); }

  cluster_vector()   { cluster.push_back( cluster_type()); }
};

#ifdef KEY // fix g++ 3.2 problems
template <class C>
  bool operator!=(C x, C y) { return !(x == y); }
#endif


struct one_dimensional_container_tag {};
struct two_dimensional_container_tag : public one_dimensional_container_tag {};

template <class Container>
struct container_traits {
  typedef one_dimensional_container_tag container_category;
};

template <class T, class Indexfunction>
struct container_traits<cluster_vector<T, Indexfunction> > {
  typedef two_dimensional_container_tag  container_category;
};

template <class Container>
inline typename container_traits<Container>::container_category
container_category(const Container&) {
  typedef typename container_traits<Container>::container_category category;
  return category();
}



//************************************************************************
//  Generic graph utilities
//************************************************************************

template <class Graph, class Edge>
Edge& add_edge(Graph& g, const Edge& e)
{
  g.extend(first(e));
  g.extend(second(e));
  return *(g.insert(e));
}

template <class Graph, class Forward_Iterator>
void add_edge(Graph& g, Forward_Iterator first, Forward_Iterator last)
{
  while (first != last) add_edge(g, *first++);
}

template <class Graph, class Vertex>
typename Graph::value_type *find_edge(Graph& g, Vertex from, Vertex to)
{
  if (from >= g.size())
    return NULL;
  for (typename Graph::fast_iterator e2 = g[from].begin();
       e2 != g[from].end();
       ++e2) {
    if ((*e2).second == to) {
      return &(*e2);
    }
  }
  return NULL;
}

template <class Graph, class Vertex>
bool erase_edge(Graph& g, Vertex from, Vertex to)
{
  if (from >= g.size())
    return false;
  for (typename Graph::fast_iterator e2 = g[from].begin();
       e2 != g[from].end();
       ++e2) {
    if ((*e2).second == to) {
      g[from].erase(e2);
      return true;
    }
  }
  return false;
}


template <class Graph_in, class Graph_out>
void reverse_graph(Graph_in& in, Graph_out& out, one_dimensional_container_tag)
{
  // copy the edges
  for (typename Graph_in::iterator e = in.begin(); e != in.end(); ++e)
    add_edge(out, edge(second(*e), first(*e)));
}

template <class Graph_in, class Graph_out>
void reverse_graph(Graph_in& in, Graph_out& out, two_dimensional_container_tag)
{
  // copy the edges
  for (typename Graph_in::cluster_iterator ci = in.cluster_begin();
       ci != in.cluster_end();
       ++ci) {
    for (typename Graph_in::fast_iterator fi = (*ci).begin(); 
	 fi != (*ci).end();
	 ++ci)
      add_edge(out, edge(second(*fi), first(*fi)));
  }
}


template <class Graph_in, class Graph_out>
inline void reverse_graph(Graph_in& in, Graph_out& out)
{
  reverse_graph(in, out, container_category(in));
}


extern INT32 Current_PU_Count();
extern char *Current_PU_Name();

template <class Graph, class Vertex_id, class Insert_iterator, class Visited_set>
void generate_post_order(Graph& g, Vertex_id v, Insert_iterator& ii, Visited_set& visited)
{
  if (visited.find(v) != visited.end()) return;
  visited.insert(v);
  for (typename Graph::fast_iterator e = g[v].begin(); 
       e != g[v].end();
       ++e) {
    generate_post_order(g, second(*e), ii, visited);
  };

  *ii++ = v;
}

typedef int vertex_id;

template <class Graph, class Vertex_id, class Container>
void generate_post_order(Graph& g, Vertex_id root, Container& c)
{
  c.erase(c.begin(), c.end());
  std::insert_iterator<Container> ii(c, c.begin());
  std::set<Vertex_id> visited;
  generate_post_order(g, root, ii, visited);

  {
    typename Container::size_type middle = c.size();  // cannot use iterator because of realloc

    // scan 'g' to catch all disconnected components
    for (vertex_id i = 0; i < g.size(); ++i) {
      if (visited.find(i) == visited.end() && g[i].size() > 0) {
	generate_post_order(g, i, ii, visited);
      }
    }

    // maintain that 'root' is the last element in the sequence
    if (c.size() > middle)
      rotate(c.begin(), c.begin()+middle, c.end());
  }
}


template <class Graph, class Vertex_id>
void find_reachable_vertex_set(Graph& g, Vertex_id v, vector<bool>& reachable)
{
  if (reachable[v]) return;
  reachable[v] = true;
  for (typename Graph::fast_iterator e = g[v].begin(); 
       e != g[v].end();
       ++e) {
    find_reachable_vertex_set(g, second(*e), reachable);
  };
}


template <class Graph_in, class Graph_out>
void subgraph(Graph_in& in, Graph_out& out, vector<bool>& vertex_set)
{
  for (typename Graph_in::iterator e = in.begin(); e != in.end(); ++e)
    if (vertex_set[first(*e)] && vertex_set[second(*e)])
      add_edge(out, *e);
}


template <class Graph_in, class Graph_out>
void copy(Graph_in& in, Graph_out& out)
{
  for (typename Graph_in::iterator e = in.begin(); e != in.end(); ++e)
    add_edge(out, *e);
}


template <class Graph>
void erase(Graph& g)
{
  for (typename Graph::cluster_id i = 0; i < g.size(); ++i) {
    g[i].erase(g[i].begin(), g[i].end());
  }
}


template <class Graph, class Vertex_id, class Container>
inline void generate_reverse_post_order(Graph& g, Vertex_id root, Container& c)
{
  generate_post_order(g, root, c);
  reverse(c.begin(), c.end());
  Is_True(c[0] == root, ("generate_reverse_post_order: root is not first element"));
}


// Topological sort for DAG == reverse post order
//
template <class Graph, class Vertex_id, class Container>
inline void topological_sort(Graph& in, Vertex_id root, Container& out)
{
  typedef typename Graph::value_type edge;
  typedef cluster_vector<edge, std::_Select1st<edge> > succ_graph;
  succ_graph g;
  copy(in, g);
  if (root < g.size())
    generate_reverse_post_order(g, root, out);
}


template <class Graph>
void print_nodes(Graph& g, FILE *fp=stdout)
{
#ifdef KEY /* Mac port */
  fprintf(fp, "number of nodes %ld: ", (long) g.size());
#else /* KEY Mac port */
  fprintf(fp, "number of nodes %d: ", (INT)g.size());
#endif /* KEY Mac port */
  for (typename Graph::cluster_iterator n = g.cluster_begin(); 
       n != g.cluster_end();
       ++n) {
    if ((*n).size() > 0)
      fprintf(fp, "%d ", g.cluster_index(n));
  }
  fprintf(fp, "\n");
}


template <class Graph>
void print_edges(Graph& g, FILE *fp = stdout)
{
  fprintf(fp, "edges: ");
  for (typename Graph::iterator e = g.begin(); e != g.end(); ++e)
    fprintf(fp, "(%d,%d) ", first(*e), second(*e));
  fprintf(fp, "\n");
}


//************************************************************************
//  Specific graph implementation :
//
//   -- vertex (cluster) are identified by an unique integers
//   -- there is one root vertex
//   -- there is one or more leaf vertices
//   -- linear time operation to access the container of all successors of a vertex
//
//************************************************************************


struct vertex {
  vertex_id id;
  int       bb_id;
  vertex(vertex_id v, vertex_id clone_from):id(v),bb_id(clone_from) {};
};

struct edge {
  typedef edge self;
  typedef vertex_id first_type;
  typedef vertex_id second_type;
  vertex_id first;
  vertex_id second;
  bool   must_fall_thru;  // is this the fall-thru edge
  friend bool operator<(const self& x, const self& y) {
    if (x.first < y.first) return true;
    if (x.first == y.first && x.second < y.second) return true;
    return false;
  }
  edge(vertex_id v, vertex_id w):first(v), second(w), must_fall_thru(false) {};
  edge(vertex_id v, vertex_id w, bool f):first(v), second(w), must_fall_thru(f) {};
};


#ifdef FUNCTION_PARTIAL_SPECIALIZATION_SUPPORT

template <class Edge>
Edge& add_edge(vector<Edge>& g, const Edge& e)
{
  g.push_back(e);
  return *(g.end()-1);
}

#else    
// workaround partial specialization problems with non-template functions.

inline edge& add_edge(vector<edge>& g, const edge& e)
{
  g.push_back(e);
  return *(g.end()-1);
}

#endif


inline vertex_id first(edge e)  { return e.first; }
inline vertex_id second(edge e) { return e.second; }

typedef cluster_vector<edge, std::_Select1st<edge> > successor_graph;
typedef cluster_vector<edge, std::_Select2nd<edge> > predecessor_graph;


template <class T>
struct fp_print : public std::unary_function<T, int> {
  FILE *fp;
  fp_print(FILE *f):fp(f) {}
  int operator()(const T& x) const { return x.print(fp); } 
};


//************************************************************************
//
//  Control flow optimization is divided into three parts: 
//    1. zone generation
//    2. zone selection
//    3. graph update.
//
//  'zone' is the data structure to specify the 
//  edges to clone.  Note that the basic
//  block set is determined by the edge set.
//  It is possible that
//  an edge is not cloned (but its head and tail are cloned).
//
//  A zone is defined by 3 kinds of edges:
//   -- entry/side_entry - edges entering the zone;
//   -- clone - edges within the zone;
//   -- exit - edges exiting the zone.
//
//  The edges entering the zone can further be divided into entry and
//  side_entry.  The zone can be divided into two by cloning and then
//  removing the entry edges from one clone, and removing the side_entry
//  edges into the other clone.
//
//  There is an implementation issue when zones overlap.   Cloning of 
//  overlapped zone is ambigous because different results generated with
//  different ordering.  When two zones overlaps, we can choose to 
//  skip the lower priority zone, or try to merge two zones if they're
//  compatible (see can_be_merged and no_bad_interference in opt_cfg_trans.cxx).
//
//  The update part performs a clone
//  of each zone and connects the entry/exit of the zone with
//  the generic graph.  Then performs an unreachable block elimination.
//  Find out who is the owner of the original basic block, and 
//  clone the basic blocks that has multiple owners.
//
//************************************************************************


static int unique_bb_count(vector<edge>& clone, vector<edge>& exit)
{
  std::set<vertex_id> visited;
  int count = 0;
  int i;
  for (i = 0; i < clone.size(); ++i) {
    vertex_id target = second(clone[i]);
    if (visited.find(target) == visited.end()) {
      ++count;
      visited.insert(target);
    }
  }
  for (i = 0; i < exit.size(); ++i) {
    vertex_id target = first(exit[i]);
    if (visited.find(target) == visited.end()) {
      ++count;
      visited.insert(target);
    }
  }
  return count;
}


struct zone {
  typedef   vector<edge>  edge_container;
  typedef   edge_container::iterator iterator;
  int id;
  int merged_into;
  bool skip;
  vertex_id loop_butterfly;

  edge_container  entry;       // clone tail, redirect edge
  edge_container  clone;       // clone head, tail, edge
  edge_container  exit;        // clone head, edge
  edge_container  side_entry;  // no change

  int profit;
  int code_expansion_saved;
  int code_expansion() {
    if (code_expansion_saved == 0)
      code_expansion_saved = unique_bb_count(clone,exit);
    return code_expansion_saved;
  }
  double priority() { 
    return ((double)profit)/code_expansion(); 
  }
  void canonicalize() {
    sort(entry.begin(), entry.end());
    sort(clone.begin(), clone.end());
    sort(exit.begin(), exit.end());
    sort(side_entry.begin(), side_entry.end());
  }
  void print(FILE *);
  zone(int i):
    id(i), merged_into(i), profit(1), code_expansion_saved(0),
    loop_butterfly(0), skip(false) {}
};


typedef vector<zone>  zone_container;
typedef zone_container::iterator zone_iterator;



// "path" is a data structure to represent a path in the graph
//  -- used in loop butterfly and feedback update procedures
//
struct path_type {
  vector<int> bbs;
  double      wt;
  vertex_id first_bb() const { return *(bbs.begin()); }
  vertex_id last_bb() const { return *(bbs.end()-1); }
  void add_bb(vertex_id v) { bbs.push_back(v); }

  path_type() {}
  path_type(vertex_id v, double w):wt(w) { add_bb(v); }
  path_type(path_type& path):bbs(path.bbs),wt(path.wt) {}
};

#ifndef KEY // workaround g++ 3.2 problem
struct less<path_type*> {
  bool operator()(path_type *p1, path_type *p2) {
    return (*p1).wt < (*p2).wt;
  }
};
#endif

extern void print_path_type(path_type *, FILE *);
extern void print_vertex_set(std::set<vertex_id> *, FILE *);
extern INT32 Current_PU_Count(void);


class COMP_UNIT;  // forward declaration

// Functions to form restructuring zones

void generate_conditional_const_zones(COMP_UNIT *cu, successor_graph &g,
				      zone_container& zones, bool trace);
void generate_loop_butterfly_zones(COMP_UNIT *cu, successor_graph &g,
				   zone_container& zones, int, bool trace);

// forward declaration.
class PRO_LOOP_FUSION_TRANS;

// bit mask for if-merging actions.
typedef enum IF_MERGE_ACTION {
  DO_NONE = 0x0,
  DO_IFMERGE = 0x1,
  DO_IFCOLLAPSE = 0x2
};

// bit mask for if-merging pass
typedef enum IF_MERGE_PASS {
  PASS_NONE = 0x0,
  PASS_GLOBAL = 0x1,
  PASS_LOCAL = 0x2
};

class PRO_LOOP_INTERCHANGE_TRANS;

class IF_MERGE_TRANS {
friend class PRO_LOOP_INTERCHANGE_TRANS;
private:
  COMP_UNIT * _cu;
  BOOL _trace;
  BOOL _dump;
  int _count;
  MAP * _val_map;   // map from an interger to a value number.
  BS * _true_val;   // a bit set of TRUE values, scratch field.
  MEM_POOL * _pool;
  PRO_LOOP_FUSION_TRANS * _tail_dup;
  IF_MERGE_ACTION _action;
  IF_MERGE_PASS _pass;
  MAP * _invar_map;  // hash BB_NODE Id to SC_NODE *
  MAP * _def_cnt_map; // hash aux id to def count
  int _region_id;    // Id of currently processed region.
private:
  void Delete_val_map();
  void Init_val_map(WN *, BOOL);
  BOOL Val_mod(SC_NODE *, WN *, BOOL);
  BOOL Val_match(WN *);
  BOOL Is_trackable_var(AUX_ID);
  BOOL Is_trackable_expr(WN *);
  void Track_val(BB_NODE *, BB_NODE *, WN *);
  void Track_val(SC_NODE *, BB_NODE *, WN *);
  AUX_ID Get_val(AUX_ID);
  void Set_val(AUX_ID, AUX_ID);
  void Remove_val(WN *, WN *);
  BOOL Is_if_collapse_cand(SC_NODE * sc1, SC_NODE * sc2);
  BOOL Maybe_assigned_expr(WN *, WN *);
  BOOL Maybe_assigned_expr(SC_NODE *, WN *, BOOL);
  BOOL Maybe_assigned_expr(BB_NODE *, WN *);
  BOOL Maybe_assigned_expr(SC_NODE *, BB_NODE *);
  BOOL Maybe_assigned_expr(SC_NODE *, SC_NODE *);
  BOOL Maybe_assigned_expr(BB_NODE *, BB_NODE *);  
  BOOL Maybe_assigned_expr(BB_NODE *, SC_NODE *);
  MAP * Get_invar_map() { return _invar_map ; }
  void New_invar_map();
  void Delete_invar_map();
  void Delete_branch(BB_NODE *);
public:
  void      Top_down_trans(SC_NODE * sc);
  BOOL      Is_cand(SC_NODE *, SC_NODE *, BOOL);
  SC_NODE * Do_merge(SC_NODE *, SC_NODE *);
  void      Set_trace(BOOL i)                { _trace = i; }
  void      Set_dump(BOOL i)                 { _dump = i; }
  PRO_LOOP_FUSION_TRANS *  Get_tail_dup()    { return _tail_dup; }
  void      Set_tail_dup(PRO_LOOP_FUSION_TRANS * i) { _tail_dup = i; }
  void      Set_pass(IF_MERGE_PASS i) { _pass = i; }
  void      Set_count(int i) { _count = i; }
  void      Set_region_id(int i) { _region_id = i; }

  IF_MERGE_TRANS(void) { Clear(); }
  IF_MERGE_TRANS(COMP_UNIT * i) { Clear(); _cu = i; }
  IF_MERGE_TRANS(const IF_MERGE_TRANS&);
  ~IF_MERGE_TRANS(void) {};

  void Clear(void);
  int Count() { return _count; }
  void Inc_count() { _count ++; }
  void Set_pool(MEM_POOL * i) { _pool = i; };
  void Merge_CFG(SC_NODE *, SC_NODE *);
  void Merge_SC(SC_NODE *, SC_NODE *);
  BOOL Has_dependency(SC_NODE *, SC_NODE *);
  BOOL Has_dependency(SC_NODE *, BB_NODE *);
  BOOL Has_dependency(BB_NODE *, BB_NODE *);
  BOOL Has_dependency(SC_NODE * , WN *);
  BOOL Has_dependency(BB_NODE *, WN *);
  BOOL Is_aliased(WN *, WN *);
  BOOL Can_be_speculative(SC_NODE *);
  BOOL Can_be_speculative(BB_NODE *);
  BOOL Can_be_speculative(WN *);
  BOOL Is_executable_stmt(OPERATOR i) { return ((i != OPR_PRAGMA) && (i != OPR_LABEL)); }
  BOOL Has_side_effect(WN *);
  void Hash_def_map(SC_NODE *);
  void Hash_def_map(BB_NODE *);
  void Delete_def_map();
  unsigned long Get_def_cnt(AUX_ID i) 
      { return (unsigned long) _def_cnt_map->Get_val((POINTER) i); }
  ST * Get_st(WN *);
};

class PRO_LOOP_FUSION_TRANS {
private:
  COMP_UNIT * _cu;
  BOOL _trace;
  BOOL _dump;
  int _transform_count;
  IF_MERGE_TRANS * _if_merge;
  MEM_POOL * _pool;
  MAP  *_loop_depth_to_loop_map;  // map from SC tree depth to a list of SC_LOOPs, scratch field
  SC_LIST * _loop_list;          // a list of SC_LOOPs, scratch field
  int _last_class_id;
  BOOL _edit_loop_class;
  INT32 _code_bloat_count;
private:
  void Reset_loop_class(SC_NODE *, int);
  void Find_loop_class(SC_NODE *);
  void Collect_classified_loops(SC_NODE *);
  BOOL Is_cand_type(SC_TYPE type) { return ((type == SC_IF) || (type == SC_LOOP)); }
  void Find_cand(SC_NODE *, SC_NODE **, SC_NODE **, SC_NODE *);
  BOOL Traverse_trans(SC_NODE *, SC_NODE *);
  void Insert_region(BB_NODE *, BB_NODE *, BB_NODE *, BB_NODE *, MEM_POOL *);
  BOOL Is_delayed(SC_NODE *, SC_NODE *);
  void Fix_parent_info(SC_NODE *, SC_NODE *);
  BOOL Is_worthy(SC_NODE *);
public:
  void Clear();
  PRO_LOOP_FUSION_TRANS(void) { Clear(); }
  PRO_LOOP_FUSION_TRANS(COMP_UNIT * i) { Clear(); _cu = i; }
  void Set_trace(BOOL i) { _trace = i; }
  void Set_dump(BOOL i)  { _dump = i; }
  void Set_if_merge(IF_MERGE_TRANS * i) { _if_merge = i; }
  void Set_pool(MEM_POOL * i) { _pool = i; }
  MEM_POOL * Loc_pool() { return _pool; }
  int Transform_count() { return _transform_count; }
  void Inc_transform_count() { _transform_count++; }
  void Top_down_trans(SC_NODE *);
  void Nonrecursive_trans(SC_NODE *, BOOL);
  void Classify_loops(SC_NODE *);
  int New_class_id() { _last_class_id ++; return _last_class_id; }
  void Do_code_motion(SC_NODE *, SC_NODE *);
  void Do_head_duplication(SC_NODE *, SC_NODE *);
  void Do_tail_duplication(SC_NODE *, SC_NODE *);
};

// bit mask for proactive loop interchange actions.
typedef enum PRO_LOOP_INTERCHANGE_ACTION {
    DO_INTERCHANGE_NONE = 0x0,
    DO_TREE_HEIGHT_RED = 0x1,  // do if-condition tree height reduction
    DO_IF_COND_DIS = 0x2,  // do if-condition distribution
    DO_REV_LOOP_UNS = 0x4,  // do reverse loop-unswitching
    DO_LOOP_UNS = 0x8  // do loop-unswitching
};

typedef enum CANON_ACTION {
    CANON_NONE = 0x0,
    SPLIT_IF_HEAD = 0x1, // Split head of SC_IF so that it only contains one statement.
    HEAD_DUP = 0x2, // Head duplicate preceding siblings of SC_IF's head.
    TAIL_DUP = 0x4  // Tail duplicate SC_IF's merge.
};

class PRO_LOOP_INTERCHANGE_TRANS {
private:
    COMP_UNIT * _cu;
    BOOL _trace;
    BOOL _dump;
    int _transform_count;
    IF_MERGE_TRANS * _if_merge;
    MEM_POOL * _pool;
    STACK<SC_NODE *> * _outer_stack; // a stack of outer SC_LOOPs. 
    STACK<SC_NODE *> * _inner_stack; // a stack of inner SC_LOOPs. 
    STACK<SC_NODE *> * _local_stack; // scratch field.
    STACK<SC_NODE *> * _restart_stack; // a stack of SC_NODEs where restarting occurs.
    STACK<SC_NODE *> * _tmp_stack; // scratch field.
    int _action;
    MAP * _def_map; //  Map symbol auxiliary Id to definition WN *.
    STACK<SC_NODE *> * _unlink_sc; // A stack of unlinked SC_NODE *.
private:
    int Nonrecursive_trans(SC_NODE *, SC_NODE *);
    BOOL Is_candidate(SC_NODE *, SC_NODE *);
    BOOL Is_invariant(SC_NODE *, SC_NODE *, AUX_ID);
    BOOL Is_invariant(SC_NODE *, BB_NODE *, AUX_ID);
    BOOL Is_invariant(BB_NODE *, BB_NODE *, AUX_ID);
    BOOL Is_invariant(BB_NODE *, WN * wn, AUX_ID);
    BOOL Is_invariant(SC_NODE *, WN * wn, AUX_ID);
    WN * Get_cond(SC_NODE *, BOOL);
    WN * Merge_cond(WN *, WN *, OPERATOR);
    BOOL Do_if_cond_tree_height_reduction(SC_NODE *, SC_NODE *);
    BOOL Do_loop_unswitching(SC_NODE *, SC_NODE *);
    BOOL Do_reverse_loop_unswitching(SC_NODE *, SC_NODE *, SC_NODE *);
    SC_NODE * Do_loop_dist(SC_NODE *, BOOL);
    BOOL Do_if_cond_dist(SC_NODE *);
    BOOL Can_reorder_cond(WN *, WN *);
    void Do_hoist(SC_NODE * sc1, SC_NODE * sc2);
    void Do_if_cond_wrap(BB_NODE *, SC_NODE *, BOOL);
    BOOL Check_sibling(SC_NODE *, SC_NODE *);
    void Do_canon(SC_NODE *, SC_NODE *, int);
    void Hash_invar(BB_NODE *, SC_NODE *);
    void Remove_block(SC_NODE *);
    void Remove_block(BB_NODE *);
    void Invalidate_invar(SC_NODE *);
    void Invalidate_invar(BB_NODE *);
    SC_NODE * Split(SC_NODE *, SC_NODE *);
    SC_NODE * Do_pre_dist(SC_NODE *, SC_NODE *);
    SC_NODE * Do_partition(SC_NODE *);
    SC_NODE * Do_loop_fusion(SC_NODE *);
    void Do_lock_step_fusion(SC_NODE *, SC_NODE *);
    BOOL Do_loop_interchange(SC_NODE *, SC_NODE *);
    void Swap_stmt(BB_NODE *, BB_NODE *);
    void Add_def_map(AUX_ID, WN *);
    void Copy_prop(SC_NODE * sc);
    void Copy_prop(BB_NODE * bb);
    void Copy_prop(WN *);
    BOOL Get_unique_ref(SC_NODE *, SC_NODE *, WN **);
    BOOL Get_unique_ref(BB_NODE *, SC_NODE *, WN **);
    BOOL Get_unique_ref(WN *, SC_NODE *, WN **);
    BOOL Compare_trees(WN *, SC_NODE *, WN *, SC_NODE *);
    BOOL Hoist_succ_blocks(SC_NODE *);
    WN * Get_index_load(SC_NODE *);
    WN * Get_index_load(WN *, ST *);
    BOOL Is_perfect_loop_nest(SC_NODE *);
    BOOL Check_iteration(SC_NODE *, SC_TYPE, SC_NODE *);
    BOOL Check_index(SC_NODE *);
    SC_NODE * Find_fusion_buddy(SC_NODE *, STACK<SC_NODE *> *);
    void Do_split_if_head(SC_NODE *);
    SC_NODE * Find_dist_cand(SC_NODE *);
    BOOL Can_interchange(SC_NODE *, SC_NODE *);
    BOOL Can_fuse(SC_NODE *);
public:
    void Clear();
    PRO_LOOP_INTERCHANGE_TRANS(void) { Clear(); }
    PRO_LOOP_INTERCHANGE_TRANS(COMP_UNIT * i) { Clear(); _cu = i; }
    void Set_trace(BOOL i) { _trace = i; }
    void Set_dump(BOOL i)  { _dump = i; }
    void Set_if_merge(IF_MERGE_TRANS * i) { _if_merge = i; }
    void Set_pool(MEM_POOL * i) { _pool = i; }
    MEM_POOL * Loc_pool() { return _pool; }
    int Transform_count() { return _transform_count; }
    void Inc_transform_count() { _transform_count++; }
    BOOL Top_down_trans(SC_NODE *, BOOL);
};

#endif
