#ifndef RESOLVER_H_H
#define RESOLVER_H_H

#include <list>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <llvm/Pass.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Analysis/MemoryDependenceAnalysis.h>

namespace lle{
   struct NoResolve;
   struct UseOnlyResolve;
   class MDAResolve;
   class ResolverBase;
   template<typename Impl>
   class Resolver;

   typedef std::tuple<
      std::unordered_set<llvm::Value*>, // all resolved value
      std::list<llvm::Value*>, // all unresolved value
      std::unordered_map<llvm::Value*, llvm::Instruction*> // some part deep_resolve map
         >
      ResolveResult;
	typedef std::pair<llvm::MemDepResult,llvm::BasicBlock*> FindedDependenciesType;
};

/**
 * provide a stright forward deep resolve
 */
struct lle::NoResolve
{
   llvm::Instruction* operator()(llvm::Value*);
};

/**
 * Use llvm User and Use to provide deep resolve
 */
struct lle::UseOnlyResolve
{
   llvm::Instruction* operator()(llvm::Value*);
};

/**
 * Use llvm MemoryDependencyAnalysis to provide deep resolve
 * useage : Resolver<MDAResolve> R;
 *          R.get_impl().initial(this); //<--- initial with Pass
 *          R.resolve(...);
 *
 * note   : this class is not implement well, add need rewrite
 */
class lle::MDAResolve
{
   llvm::Pass* pass;

   public:
   MDAResolve(){pass = NULL;}
   void initial(llvm::Pass* pass){ this->pass = pass;}
   llvm::Instruction* operator()(llvm::Value*);

   /** use this to find a possible dep results.
    * which may clobber, def, nonlocal, nonfunclocal
    * and do further judgement
    */
   static void find_dependencies( llvm::Instruction* , 
         const llvm::Pass* ,
         llvm::SmallVectorImpl<lle::FindedDependenciesType>& ,
         llvm::NonLocalDepResult* NLDR = NULL);
};

class lle::ResolverBase
{
   bool stop_resolve = false; // stop resolve immediately
   static std::list<llvm::Value*> direct_resolve(
         llvm::Value* V, 
         std::unordered_set<llvm::Value*>& resolved,
         std::function<void(llvm::Value*)>
         );

   virtual llvm::Instruction* deep_resolve(llvm::Instruction* I) = 0;

   protected:
   // hash map, provide quick search Value* ---> Value*
   std::unordered_map<llvm::Value*, llvm::Instruction*> PartCache;

   public:
   static void _empty_handler(llvm::Value*);
   
   // walk through V's dependent tree and callback
   ResolveResult resolve(llvm::Value* V, std::function<void(llvm::Value*)> = _empty_handler);
   // walk though V's dependent tree and callback
   // if lambda return true, immediately stop resolve
   // and return true
   bool resolve_if(llvm::Value* V, std::function<bool(llvm::Value*)> lambda);

};

template<typename Impl = lle::NoResolve>
class lle::Resolver: public lle::ResolverBase
{
   Impl impl;
   llvm::Instruction* deep_resolve(llvm::Instruction* I)
   {
      llvm::Instruction* Ret = NULL;
      auto Ite = PartCache.find(I);
      if(Ite != PartCache.end())
         return Ite->second;
      Ret = impl(I);
      PartCache.insert(std::make_pair(I, Ret));
      return Ret;
   }
   public:
   Impl& get_impl(){ return impl;}
};



#endif
