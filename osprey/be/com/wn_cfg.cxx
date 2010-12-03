#include "wn_cfg.h"
#include "cfg_util.h"
#include "ir_reader.h"

// VCG helper utilities
#include <sstream>
extern void help_image_wn(std::stringstream &ss, WN *wn, INT indent);
extern void help_image_stmt(std::stringstream &ss, WN * wn, INT indent);

namespace CFG_UTIL {

//===================================================================
// WN_STMT_CONTAINER
//===================================================================
void
WN_STMT_CONTAINER::Add_stmt(WN* stmt) {
  Is_True(stmt != NULL, ("can not add NULL stmt"));
  if (_first_stmt == NULL) {
    _first_stmt = stmt;
    _last_stmt = stmt;
  }
  else if (WN_next(_last_stmt) == stmt) {
    _last_stmt = stmt;
  }
  else {
    // place the stmt into _extra_stmt
    FmtAssert(WN_next(_last_stmt) == NULL, ("WN_next(_last_stmt) must be NULL"));
    FmtAssert(WN_operator(stmt) == OPR_STID || WN_operator(stmt) == OPR_DO_WHILE,
              ("extra expr can only be STID(for DO_LOOP step) or DO_WHILE"));
    FmtAssert(_extra_stmt == NULL, ("extra stmt must be NULL"));
    _extra_stmt = stmt;
  }
}

WN*
WN_STMT_CONTAINER::First_stmt() const {
  return _first_stmt;
}

WN*
WN_STMT_CONTAINER::Next_stmt(WN* stmt) const {
  Is_True(stmt != NULL, ("stmt is NULL"));
  if (stmt == _last_stmt)
    return _extra_stmt;
  else if (stmt == _extra_stmt)
    return NULL;  // no next
  else
    return WN_next(stmt);
}

WN*
WN_STMT_CONTAINER::Prev_stmt(WN* stmt) const {
  Is_True(stmt != NULL, ("stmt is NULL"));
  if (stmt == _extra_stmt)
    return _last_stmt;
  else if (stmt == _first_stmt)
    return NULL;
  else
    return WN_prev(stmt);
}

WN*
WN_STMT_CONTAINER::Last_stmt() const {
  if (_extra_stmt != NULL) {
    Is_True(_last_stmt != NULL && WN_next(_last_stmt) == NULL, ("extra stmt is wrong"));
    Is_True(WN_operator(_extra_stmt) == OPR_STID || WN_operator(_extra_stmt) == OPR_DO_WHILE, 
            ("extra stmt is not STID(incr of DO_LOOP) or DO_WHILE"));
    return _extra_stmt;
  }
  else {
    return _last_stmt;
  }
}

BOOL
WN_STMT_CONTAINER::Is_empty() const {
  return _first_stmt == NULL;
}

void
WN_STMT_CONTAINER::Print(FILE* fp, INT32 dump_flag) const {
  if (dump_flag & DUMP_AIR) {
    for (WN* wn = First_stmt(); wn != NULL; wn = Next_stmt(wn)) {
      if (dump_flag & DUMP_EXPR) {
        fdump_tree(fp, wn);
      }
      else {
        fdump_wn(fp, wn);
      }
    }
  }
}

VCGNode*
WN_STMT_CONTAINER::VCG_dump(MEM_POOL* mpool, VCGGraph& vcg, INT32 dump_flag) const {
  VCGNode* bb_node = NULL;
  if (dump_flag & DUMP_AIR) {
    std::stringstream ss;
    for (WN* wn = _first_stmt; wn != NULL; wn = Next_stmt(wn)) {
      if (dump_flag & DUMP_EXPR) {
        help_image_stmt(ss, wn, 0);
      }
      else {
        help_image_wn(ss, wn, 0);
      }
    }
    char* label = (char *) MEM_POOL_Alloc(mpool, ss.str().size()+1);
    strcpy(label, ss.str().c_str());
    bb_node = CXX_NEW(VCGNode(NULL, label), mpool); // title will be set later
    vcg.addNode(*bb_node);
  }
  return bb_node;
}

//===================================================================
// WN_STMT_MAPPER
//===================================================================
void
WN_STMT_MAPPER::Connect_stmt_node(STMT stmt, BB_NODE* node) {
  Is_True(_stmt_map.find((INTPTR)stmt) == _stmt_map.end(), ("stmt has been associated with a node"));
  _stmt_map[(INTPTR)stmt] = node;
}

void
WN_STMT_MAPPER::Disconnect_stmt_node(STMT stmt, BB_NODE* node) {
  STMT_MAP::iterator it = _stmt_map.find((INTPTR)stmt);
  Is_True(it != _stmt_map.end(), ("stmt does not have a node"));
  _stmt_map.erase(it);
}

void
WN_STMT_MAPPER::Disconnect_all_stmt_in_node(BB_NODE* node) {
  for (BB_NODE::stmt_iterator it = node->Stmt_begin();
       it != node->Stmt_end();
       ++it) {
    Disconnect_stmt_node(&(*it), node);
  }
}

WN_STMT_MAPPER::BB_NODE*
WN_STMT_MAPPER::Get_stmt_node(STMT stmt) const {
  STMT_MAP::const_iterator it = _stmt_map.find((INTPTR)stmt);
  if (it != _stmt_map.end())
    return it->second;
  else
    return NULL;
}

//===================================================================
// WN_CFG
//===================================================================
void
WN_CFG::Set_parent(WN* parent, WN* kid) {
  Is_True(parent != NULL && kid != NULL, ("Parent or kid is NULL"));
  Is_True(_parent_map.find((INTPTR)kid) == _parent_map.end(), 
          ("Kid already has a parent"));
#ifdef Is_True_On
  int i;
  for (i=0; i<WN_kid_count(parent); ++i) {
    if (WN_kid(parent, i) == kid)
      break;
  }
  FmtAssert(WN_kid_count(parent) == 0 || i != WN_kid_count(parent), 
            ("Parent and kid mismatch"));
#endif

  _parent_map[(INTPTR)kid] = parent;
}

void
WN_CFG::Set_label(INT32 label_num, BB_NODE* node) {
  Is_True(node != NULL, ("node is NULL"));
  Is_True(_label_map.find(label_num) == _label_map.end(), ("label already exists"));
  _label_map[label_num] = node;
}

WN*
WN_CFG::Get_parent(WN* kid) const {
  if (WN_operator(kid) == OPR_FUNC_ENTRY) {
    return NULL;
  }
  PARENT_MAP::const_iterator it = _parent_map.find((INTPTR)kid);
  Is_True(it != _parent_map.end(), ("Can not find parent"));
  return it->second;
}

WN*
WN_CFG::Get_parent_stmt(WN* wn) const {
  Is_True(WN_operator(wn) != OPR_FUNC_ENTRY, ("No parent for FUNC_ENTRY"));
  if (OPERATOR_is_stmt(WN_operator(wn))) {
    return wn;
  }
  if (!OPERATOR_is_expression(WN_operator(wn))) {
    return NULL;
  }
  WN* stmt = wn;
  do {
    stmt = Get_parent(stmt);
  } while (stmt != NULL && 
           !OPERATOR_is_stmt(WN_operator(stmt)) && 
           !OPERATOR_is_scf(WN_operator(stmt)));
  Is_True(stmt != NULL && 
          (OPERATOR_is_stmt(WN_operator(stmt)) || OPERATOR_is_scf(WN_operator(stmt))), 
          ("Can not find stmt"));
  return stmt;
}

WN*
WN_CFG::Get_parent_block(WN* wn) const {
  if (WN_operator(wn) == OPR_BLOCK) {
    return wn;
  }
  WN* block = wn;
  do {
    block = Get_parent(block);
  } while (block != NULL && WN_operator(block) != OPR_BLOCK);
  Is_True(block != NULL && WN_operator(block) == OPR_BLOCK, ("Can not find the block"));
  return block;
}

WN*
WN_CFG::Get_parent_scf(WN* wn) const {
  // block has the SCF property, we need to exclude OPR_BLOCK
  if (OPERATOR_is_scf(WN_operator(wn)) && WN_operator(wn) != OPR_BLOCK) {
    return wn;
  }
  WN* scf = wn;
  do {
    scf = Get_parent(scf);
  } while (scf != NULL && (!OPERATOR_is_scf(WN_operator(scf)) || WN_operator(scf) == OPR_BLOCK));
  Is_True(scf == NULL || (OPERATOR_is_scf(WN_operator(scf)) && WN_operator(scf) != OPR_BLOCK), 
          ("Errors in finding scf"));
  return scf;
}

WN*
WN_CFG::Get_parent_region(WN* wn) const {
  if (WN_operator(wn) == OPR_REGION) {
    return wn;
  }
  WN* region = wn;
  do {
    region = Get_parent(region);
  } while (region != NULL && WN_operator(region) != OPR_REGION);
  Is_True(region == NULL || WN_operator(region) == OPR_REGION, ("Errors in finding region"));
  return region;
}

WN_CFG::BB_NODE*
WN_CFG::Get_wn_node(WN* wn) const {
  Is_True(wn != NULL, ("wn is NULL"));
  OPERATOR opr = WN_operator(wn);
  if (opr == OPR_BLOCK) {
    return NULL;  // no block in CFG
  }
  else if (OPERATOR_is_expression(opr)) {
    WN* stmt = Get_parent_stmt(wn);
    Is_True(stmt != NULL, ("Can not find stmt for wn"));
    return Get_stmt_node(stmt);
  }
  else if (OPERATOR_is_stmt(opr) || OPERATOR_is_scf(opr)) {
    return Get_stmt_node(wn);
  }
  else {
    return NULL;
  }
}

WN_CFG::BB_NODE*
WN_CFG::Get_label_node(INT32 label_num) const {
  LABEL_MAP::const_iterator it = _label_map.find(label_num);
  if (it != _label_map.end()) {
    return it->second;
  }
  else {
    return NULL;
  }
}

WN*
WN_CFG::Get_branch_stmt(BB_NODE* node) const {
  INT32 succ_count = node->Get_succs_count();
  Is_True(succ_count > 0, ("node does not have successor"));
  WN* last_stmt = node->Last_stmt();
  Is_True(last_stmt != NULL, ("last stmt is NULL"));
  OPERATOR opr = WN_operator(last_stmt);
  if (succ_count == 1) {
    // goto, or fall through
    if (opr == OPR_GOTO || opr == OPR_RETURN || opr == OPR_RETURN_VAL) {
      return last_stmt;
    }
    else {
      return NULL;
    }
  }
  else if (succ_count == 2) {
    // conditional goto or SCF
    if (OPERATOR_is_scf(opr)) {
      Is_True(opr == OPR_IF || opr == OPR_WHILE_DO || 
              opr == OPR_DO_WHILE || opr == OPR_DO_LOOP,
              ("incorrent WN operator of last stmt"));
      return last_stmt;
    }
    else if (opr == OPR_TRUEBR || opr == OPR_FALSEBR ) {
      return last_stmt;
    }
    else if (OPERATOR_is_compare(opr)) {
      WN* parent = Get_parent(last_stmt);
      Is_True(node->First_stmt() == node->Last_stmt(), ("node should only contain one stmt"));
      Is_True(WN_operator(parent) == OPR_DO_LOOP, ("parent should be DO_LOOP"));
      Is_True(WN_end(parent) == last_stmt, ("last stmt should be the end of DO_LOOP"));
      return parent;
    }
    else {
      if (opr == OPR_GOTO)
        return last_stmt;
      return NULL;
    }
  }
  else {
    // switch or comp_goto
    if (opr == OPR_SWITCH || opr == OPR_COMPGOTO) {
      return last_stmt;
    }
    else {
      return NULL;
    }
  }
}

//===================================================================
// Build_CFG
//   Build CFG and parent map based on WHIRL tree
//   Handle connectivity issues in CFG
//===================================================================
void
WN_CFG_BUILDER::Build_CFG() {
  Is_True(_root != NULL, ("root wn is NULL"));
  if (WN_operator(_root) == OPR_REGION) {
    FmtAssert(FALSE, ("TODO: build CFG for region"));
  }
  Is_True(WN_operator(_root) == OPR_FUNC_ENTRY, ("root is not FUNC_ENTRY"));

  // build the CFG
  _current_bb = NULL;
  Add_entry(_cfg, _root);

  // build parent map
  // performance TODO: combine the two traversals
  Build_parent_map_stmt(_cfg, _root);

  // connect AGOTOs
  Connect_agotos();

  // disconnect unreachable nodes and connect not exit nodes to dummy exit
  CFG_CONNECTIVITY<WN_CFG> connectivity(_cfg);
  connectivity.Perform();
}

//===================================================================
// Build_parent_map_stmt
//   Build parent map for stmt
// Build_parent_map_block
//   Build parent map for block
//===================================================================
void
WN_CFG_BUILDER::Build_parent_map_stmt(WN_CFG& cfg, WN* wn) {
  Is_True(wn != NULL, ("wn is NULL"));
  Is_True(WN_operator(wn) != OPR_BLOCK, ("wn can not be BLOCK"));
  for (int i=0; i<WN_kid_count(wn); ++i) {
    WN* kid = WN_kid(wn, i);
    cfg.Set_parent(wn, kid);
    if (WN_operator(kid) != OPR_BLOCK) {
      Build_parent_map_stmt(cfg, kid);
    }
    else {
      Build_parent_map_block(cfg, kid);
    }
  }
}

void
WN_CFG_BUILDER::Build_parent_map_block(WN_CFG& cfg, WN* wn) {
  Is_True(wn != NULL, ("wn is NULL"));
  Is_True(WN_operator(wn) == OPR_BLOCK, ("wn must be BLOCK"));
  for (WN* kid = WN_first(wn); kid != NULL; kid = WN_next(kid)) {
    Is_True(WN_operator(kid) != OPR_BLOCK, ("kid can not be block"));
    cfg.Set_parent(wn, kid);
    Build_parent_map_stmt(cfg, kid);
  }
}

//===================================================================
// Connect_agotos
//   Connect the bb contains agoto to all bb with label
//   The target label should be address taken?
// Connect_goto_to_label
//   Connect the goto bbs with the label bbs
//   If the label bbs do not exist, push goto bbs into stack
//===================================================================
void
WN_CFG_BUILDER::Connect_agotos() {
  // TODO: connect agotos
}

void
WN_CFG_BUILDER::Connect_goto_to_label(WN_CFG& cfg, BB_NODE* bb, WN* wn) {
  Is_True(WN_operator(wn) == OPR_GOTO ||
          WN_operator(wn) == OPR_CASEGOTO ||
          WN_operator(wn) == OPR_REGION_EXIT, 
          ("WN is not goto or casegoto"));
  Is_True(bb != NULL, ("bb is NULL"));

  LABEL_MAP::iterator lit = _label_map.find(WN_label_number(wn));
  if (lit != _label_map.end()) {
    cfg.Connect_predsucc(bb, lit->second);
  }
  else {
    vector<BB_NODE*>& gotos = _goto_map[WN_label_number(wn)];
    gotos.push_back(bb);
  }
}

//===================================================================
// Add_entry: handle FUNC_ENTRY
//   FUNC_ENTRY --> ignore
//     [kid0..n-3] IDNAME --> ignore
//     [kidn-3] PRAGMA --> ignore
//     [kidn-2] PRAGMA --> ignore
//     [kidn-1] BODY --> BB0
//===================================================================
void
WN_CFG_BUILDER::Add_entry(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_FUNC_ENTRY,
          ("WN is not FUNC_ENTRY"));
  Is_True(_current_bb == NULL, ("Current BB is not NULL"));

  // put BODY into BB0
  _current_bb = cfg.Create_node();
  cfg.Add_entry_node(_current_bb);
  Add_block(cfg, WN_func_body(wn));
}

//===================================================================
// Add_altentry: handle ALTENTRY
//   STMT0 --> BB0 (must be GOTO/RETURN)
//   FUNC_ENTRY --> BB1 (BB1 don't have other preds than dummy_entry
//     [kid0..n-1] IDNAME --> ignore
//===================================================================
void
WN_CFG_BUILDER::Add_altentry(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_ALTENTRY,
          ("WN is not ALTENTRY"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));
  Is_True(_current_bb->Get_preds_count() == 0, ("Current BB has predecessors"));

  // put BODY into BB1
  _current_bb = cfg.Create_node();
  cfg.Add_entry_node(_current_bb);
}

//===================================================================
// Add_if: handle IF
//   STMT0 --> BB0
//   OPR_IF --> BB0
//     [kid0] COND --> BB0
//     [kid1] THEN --> BB1
//     [kid2] ELSE --> BB2
//   STMT1 --> BB3
//===================================================================
void
WN_CFG_BUILDER::Add_if(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_IF, ("WN is not IF"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put OPR_IF into BB0
  BB_NODE* if_bb = _current_bb;
  Add_stmt(cfg, if_bb, wn);

  // put THEN block into BB1
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(if_bb, _current_bb);
  Add_block(cfg, WN_then(wn));
  BB_NODE* then_bb = _current_bb;

  // put ELSE block into BB2
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(if_bb, _current_bb);
  Add_block(cfg, WN_else(wn));
  BB_NODE* else_bb = _current_bb;

  // connect THEN/ELSE block with BB3
  _current_bb = cfg.Create_node();
  if (then_bb != NULL)
    cfg.Connect_predsucc(then_bb, _current_bb);
  if (else_bb != NULL)
    cfg.Connect_predsucc(else_bb, _current_bb);
}

//===================================================================
// Add_do_loop: handle DO_LOOP
//   STMT0 --> BB0
//   OPR_DO_LOOP --> BB0 <include INIT>
//     [kid0] IDNAME --> ignore
//     [kid1] INIT --> BB0
//     [kid2] COMP --> BB1
//     [kid3] INCR --> BB2* (put into _extra field)
//     [kid4] BODY --> BB2
//     [kid5] LOOP_INFO --> ignore
//   STMT1 --> BB3
//===================================================================
void
WN_CFG_BUILDER::Add_do_loop(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_DO_LOOP, ("WN is not DO_LOOP"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put INIT into BB0
  BB_NODE* init_bb = _current_bb;
  Add_stmt(cfg, init_bb, wn);

  // put COMP into BB1
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(init_bb, _current_bb);
  BB_NODE* cmp_bb = _current_bb;
  Add_stmt(cfg, cmp_bb, WN_end(wn));

  // put BODY/INCR into BB2
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(cmp_bb, _current_bb);
  BB_NODE* body_bb = _current_bb;
  Add_block(cfg, WN_do_body(wn));
  Add_stmt(cfg, _current_bb, WN_step(wn));
  cfg.Connect_predsucc(_current_bb, cmp_bb);

  // connect with BB3
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(cmp_bb, _current_bb);
}

//===================================================================
// Add_do_while: handle DO_WHILE
//   STMT0 --> BB0
//   DO_WHILE
//     [kid0] COMP --> BB1* (put into _extra field)
//     [kid1] BODY --> BB1
//   STMT1 --> BB2
//===================================================================
void
WN_CFG_BUILDER::Add_do_while(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_DO_WHILE, ("WN is not DO_WHILE"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put BODY into BB1
  if (!_current_bb->Is_empty()) {
    // create new BB if current BB isn't empty
    BB_NODE* last_bb = _current_bb;
    _current_bb = cfg.Create_node();
    cfg.Connect_predsucc(last_bb, _current_bb);
  }
  BB_NODE* body_entry = _current_bb;
  Add_block(cfg, WN_while_body(wn));
  BB_NODE* body_exit = _current_bb;

  if (_current_bb == NULL) {
    // there is a GOTO at the end of body
    _current_bb = cfg.Create_node();
  }
  else if (_current_bb->Get_succs_count() > 0) {
    // can not put the COMP to BB1 in this case
    _current_bb = cfg.Create_node();
    cfg.Connect_predsucc(body_exit, _current_bb);
  }
  Add_stmt(cfg, _current_bb, wn);
  cfg.Connect_predsucc(_current_bb, body_entry);

  // connect BB2 and BB3
  if (WN_next(wn) != NULL) {
    BB_NODE* cmp_bb = _current_bb;
    _current_bb = cfg.Create_node();
    cfg.Connect_predsucc(cmp_bb, _current_bb);
  }
}

//===================================================================
// Add_while_do: handle WHILE_DO
//   STMT0 --> BB0
//   WHILE_DO --> BB1
//     [kid0] COMP --> BB1
//     [kid1] BODY --> BB2
//   STMT1 --> BB3
//===================================================================
void
WN_CFG_BUILDER::Add_while_do(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_WHILE_DO, ("WN is not WHILE_DO"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put COMP into BB1
  if (!_current_bb->Is_empty()) {
    // create new BB if current BB isn't empty
    BB_NODE* last_bb = _current_bb;
    _current_bb = cfg.Create_node();
    cfg.Connect_predsucc(last_bb, _current_bb);
  }
  BB_NODE* cmp_bb = _current_bb;
  Add_stmt(cfg, cmp_bb, wn);

  // put BODY into BB2
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(cmp_bb, _current_bb);
  Add_block(cfg, WN_while_body(wn));
  if (_current_bb != NULL) {
    // there is a goto at the end of body
    cfg.Connect_predsucc(_current_bb, cmp_bb);
  }

  // connect BB1 and BB3
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(cmp_bb, _current_bb);
}

//===================================================================
// Add_compgoto: handle COMP_GOTO
//   STMT0 --> BB0
//   COMPGOTO --> BB0
//     [kid0] LDID --> BB0
//     [kid1] JUMPs --> BB0
//     [kid2] GOTO --> BB0
//   STMT1 --> BB1
//===================================================================
void
WN_CFG_BUILDER::Add_compgoto(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_COMPGOTO, ("Wn is not COMPGOTO"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put all kids into BB0
  Add_stmt(cfg, _current_bb, wn);
  WN* goto_wn = WN_first(WN_switch_table(wn));
  while (goto_wn != NULL) {
    Connect_goto_to_label(cfg, _current_bb, goto_wn);
    goto_wn = WN_next(goto_wn);
  }
  if (WN_switch_default(wn) != NULL)
    Connect_goto_to_label(cfg, _current_bb, WN_switch_default(wn));

  // create BB1
  if (WN_next(wn) != NULL)
    _current_bb = cfg.Create_node();
  else
    _current_bb = NULL;    // no fall-through for compgoto
}

//===================================================================
// Add_switch: handle SWITCH
//   STMT0 --> BB0
//   SWITCH --> BB0
//     [kid0] LDID --> BB0
//     [kid1] CASEGOTOs --> BB0
//     [kid2] GOTO --> BB0
//   STMT1 --> BB1
//===================================================================
void
WN_CFG_BUILDER::Add_switch(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_SWITCH, ("Wn is not SWITCH"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put all kids into BB0
  Add_stmt(cfg, _current_bb, wn);
  WN* casegoto_wn = WN_first(WN_switch_table(wn));
  while (casegoto_wn != NULL) {
    Connect_goto_to_label(cfg, _current_bb, casegoto_wn);
    casegoto_wn = WN_next(casegoto_wn);
  }
  if (WN_switch_default(wn) != NULL)
    Connect_goto_to_label(cfg, _current_bb, WN_switch_default(wn));

  // create BB1
  if (WN_next(wn) != NULL)
    _current_bb = cfg.Create_node();
  else
    _current_bb = NULL;    // no fall-through for switch
}

//===================================================================
// Add_label: handle LABEL
//   STMT0 --> BB0
//   LABEL --> BB1
//   STMT1 --> BB1
//===================================================================
void
WN_CFG_BUILDER::Add_label(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_LABEL, ("WN is not LABEL"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  if (!_current_bb->Is_empty()) {
    // create new bb if current BB isn't empty
    BB_NODE* last_bb = _current_bb;
    _current_bb = cfg.Create_node();
    cfg.Connect_predsucc(last_bb, _current_bb);
  }
  Add_stmt(cfg, _current_bb, wn);
  FmtAssert(_label_map.find(WN_label_number(wn)) == _label_map.end(),
            ("found duplicated labels"));
  _label_map[WN_label_number(wn)] = _current_bb;
  cfg.Set_label(WN_label_number(wn), _current_bb);

  // connect GOTOs with this LABEL
  GOTO_MAP::iterator vit = _goto_map.find(WN_label_number(wn));
  if (vit != _goto_map.end()) {
    vector<BB_NODE*>& gotos = vit->second;
    for (vector<BB_NODE*>::iterator nit = gotos.begin();
         nit != gotos.end();
         ++nit) {
      cfg.Connect_predsucc(*nit, _current_bb);
    }
    // remove the gotos
    _goto_map.erase(vit);
  }
}

//===================================================================
// Add_goto: handle GOTO
//   STMT0 --> BB0
//   GOTO  --> BB0
//   STMT1 --> BB1
//===================================================================
void
WN_CFG_BUILDER::Add_goto(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_GOTO, ("WN is not goto"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put GOTO into BB0
  Add_stmt(cfg, _current_bb, wn);
  Connect_goto_to_label(cfg, _current_bb, wn);

  // create BB1
  if (WN_next(wn) != NULL)
    _current_bb = cfg.Create_node();
  else
    _current_bb = NULL;    // no fall-through for goto
}

//===================================================================
// Add_goto: handle XGOTO
//===================================================================
void
WN_CFG_BUILDER::Add_xgoto(WN_CFG& cfg, WN* wn) {
  FmtAssert(FALSE, ("TODO: handle XGOTO"));
}

//===================================================================
// Add_goto: handle AGOTO
//===================================================================
void
WN_CFG_BUILDER::Add_agoto(WN_CFG& cfg, WN* wn) {
  FmtAssert(FALSE, ("TODO: handle AGOTO"));
}

//===================================================================
// Add_condgoto: handle conditional goto, TRUEBR, FALSEBR
//   STMT0 --> BB0
//   CONDGOTO  --> BB0
//   STMT1 --> BB1
//===================================================================
void
WN_CFG_BUILDER::Add_condgoto(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_TRUEBR || WN_operator(wn) == OPR_FALSEBR,
          ("WN is not TRUEBR or FALSEBR"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put CONDGOTO into BB0
  Add_stmt(cfg, _current_bb, wn);
  BB_NODE* condgoto_bb = _current_bb;
  LABEL_MAP::iterator lit = _label_map.find(WN_label_number(wn));
  if (lit != _label_map.end()) {
    cfg.Connect_predsucc(_current_bb, lit->second);
  }
  else {
    vector<BB_NODE*>& gotos = _goto_map[WN_label_number(wn)];
    gotos.push_back(_current_bb);
  }

  // create BB1
  _current_bb = cfg.Create_node();
  cfg.Connect_predsucc(condgoto_bb, _current_bb);
}

//===================================================================
// Add_block: handle the BLOCK
//   traverse the block and take actions for different WHIRL node
//   STMT1 --> handle stmt1 based on stmt1's operator
//   STMT2 --> handle stmt2 based on stmt2's operator
//   STMT3 --> handle stmt3 based on stmt3's operator
//===================================================================
void
WN_CFG_BUILDER::Add_block(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_BLOCK, ("wn is not BLOCK"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  for (WN* stmt = WN_first(wn); stmt != NULL; stmt = WN_next(stmt)) {
    OPERATOR opr = WN_operator(stmt);
    switch (opr) {
    case OPR_IF:
      Add_if(cfg, stmt); break;
    case OPR_DO_LOOP:
      Add_do_loop(cfg, stmt); break;
    case OPR_DO_WHILE:
      Add_do_while(cfg, stmt); break;
    case OPR_WHILE_DO:
      Add_while_do(cfg, stmt); break;
    case OPR_COMPGOTO:
      Add_compgoto(cfg, stmt); break;
    case OPR_SWITCH:
      Add_switch(cfg, stmt); break;
    case OPR_ALTENTRY:
      Add_altentry(cfg, stmt); break;
    case OPR_LABEL:
      Add_label(cfg, stmt); break;
    case OPR_GOTO:
      Add_goto(cfg, stmt); break;
    case OPR_XGOTO:
      Add_xgoto(cfg, stmt); break;
    case OPR_AGOTO:
      Add_agoto(cfg, stmt); break;
    case OPR_TRUEBR:
    case OPR_FALSEBR:
      Add_condgoto(cfg, stmt); break;
    case OPR_PRAGMA:
    case OPR_XPRAGMA:
      Add_stmt(cfg, _current_bb, stmt); break;
    case OPR_REGION:
      Add_region(cfg, stmt); break;
    case OPR_REGION_EXIT:
      Add_region_exit(cfg, stmt); break;
    case OPR_BLOCK:
      Add_block(cfg, stmt); break;
    default:
      if (OPERATOR_is_stmt(opr)) {
        Add_stmt(cfg, _current_bb, stmt);
      }
      else {
        FmtAssert(FALSE, ("TODO: handle me: %s", OPCODE_name(WN_opcode(stmt))));
      }
    }
  }
}

//===================================================================
// Add_region: handle REGION
//   STMT0 --> BB0
//   REGION --> BB1
//     [kid0] REGION_EXITS --> ignore when it's not black box
//                             otherwise, connect exits with labels
//     [kid1] PRAGMAs --> ignore
//     [kid2] BODY --> BB1
//   STMT1 --> BB2
//===================================================================
void
WN_CFG_BUILDER::Add_region(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_REGION, ("wn is not REGION"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  RID *rid = REGION_get_rid(wn);
  if (rid != NULL && RID_level(rid) >= _cfg.Get_rgn_level()) {
    FmtAssert(FALSE, ("TODO: handle black-box REGION"));
  }

  if (!_current_bb->Is_empty()) {
    // Force new block if current BB isn't empty
    BB_NODE* last_bb = _current_bb;
    _current_bb = cfg.Create_node();
    cfg.Connect_predsucc(last_bb, _current_bb);
  }
  Add_block(cfg, WN_region_body(wn));
  BB_NODE* rgn_body = _current_bb;

  // create BB2
  if (WN_next(wn) != NULL) {
    _current_bb = cfg.Create_node();
    if (rgn_body != NULL) {
      cfg.Connect_predsucc(rgn_body, _current_bb);
    }
  }
}

//===================================================================
// Add_region_exit: handle REGION_EXIT
// STMT0 --> BB0
// REGION_EXIT  --> BB0
// STMT1 --> BB1
//===================================================================
void
WN_CFG_BUILDER::Add_region_exit(WN_CFG& cfg, WN* wn) {
  Is_True(WN_operator(wn) == OPR_REGION_EXIT, ("WN is not region_exit"));
  Is_True(_current_bb != NULL, ("Current BB is NULL"));

  // put GOTO into BB0
  Add_stmt(cfg, _current_bb, wn);
  Connect_goto_to_label(cfg, _current_bb, wn);

  // create BB1
  if (WN_next(wn) != NULL)
    _current_bb = cfg.Create_node();
  else
    _current_bb = NULL;    // no fall-through for region_exit
}

//===================================================================
// Add_stmt
//   Add stmt to bb
//===================================================================
void
WN_CFG_BUILDER::Add_stmt(WN_CFG& cfg, BB_NODE* bb, WN* stmt) {
  Is_True(stmt != NULL, ("wn is NULL"));

  OPERATOR opr = WN_operator(stmt);
  Is_True(opr != OPR_FUNC_ENTRY && opr != OPR_ALTENTRY, ("stmt is ENTRY"));

  bb->Add_stmt(stmt);
  cfg.Connect_stmt_node(stmt, bb);

  if (opr == OPR_RETURN || opr == OPR_RETURN_VAL) {
    cfg.Add_exit_node(bb);
    // RETURN will end current bb
    if (WN_next(stmt) != NULL)
      _current_bb = cfg.Create_node();
    else
      _current_bb = NULL;    // no fall-through for return
  }
}

} /* namespace CFG_UTIL */
