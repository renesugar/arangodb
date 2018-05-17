////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#include "ClusterIndex.h"
#include "Basics/VelocyPackHelper.h"
#include "ClusterEngine/ClusterEngine.h"
#include "Indexes/SimpleAttributeEqualityMatcher.h"
#include "StorageEngine/EngineSelectorFeature.h"
#include "VocBase/LogicalCollection.h"
#include "VocBase/ticks.h"

#include <velocypack/Collection.h>
#include <velocypack/Iterator.h>
#include <velocypack/velocypack-aliases.h>

using namespace arangodb;
using Helper = arangodb::basics::VelocyPackHelper;

ClusterIndex::ClusterIndex(TRI_idx_iid_t id, LogicalCollection* collection,
                           Index::IndexType type, VPackSlice const& info)
    : Index(id, collection, info), _type(type), _info(info) {
      TRI_ASSERT(_info.slice().isObject());
}

ClusterIndex::~ClusterIndex() {

}

void ClusterIndex::toVelocyPackFigures(VPackBuilder& builder) const {
  TRI_ASSERT(builder.isOpenObject());
  Index::toVelocyPackFigures(builder);
  // technically nothing sensible can be added here
  //builder.add(VPackObjectIterator(_indexDefinition.slice()));
  // TODO fix
}

/// @brief return a VelocyPack representation of the index
void ClusterIndex::toVelocyPack(VPackBuilder& builder, bool withFigures,
                                bool forPersistence) const {
  builder.openObject();
  Index::toVelocyPack(builder, withFigures, forPersistence);
  builder.add("unique", VPackValue(_unique));
  builder.add("sparse", VPackValue(_sparse));
  
  //static std::vector forbidden = {};
  for (auto pair : VPackObjectIterator(_info.slice())) {
    if (!pair.key.isEqualString("id") &&
        !pair.key.isEqualString("type") &&
        !pair.key.isEqualString("fields") &&
        !pair.key.isEqualString("selectivityEstimate") &&
        !pair.key.isEqualString("figures") &&
        !pair.key.isEqualString("unique") &&
        !pair.key.isEqualString("sparse")) {
      builder.add(pair.key);
      builder.add(pair.value);
    }
  }
  builder.close();
}

void ClusterIndex::updateProperties(velocypack::Slice const& slice) {
  VPackBuilder merge;
  merge.openObject();
  ClusterEngine* ce = static_cast<ClusterEngine*>(EngineSelectorFeature::ENGINE);
  if (ce->isMMFiles()) {
    // nothing to update here
  } else if (ce->isRocksDB()) {
    
    merge.add("cacheEnabled", VPackValue(Helper::readBooleanValue(slice, "cacheEnabled", false)));
    
  } else {
    TRI_ASSERT(false);
    THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
  }
  
  merge.close();
  _info = VPackCollection::merge(_info.slice(), merge.slice(), true);
}

bool ClusterIndex::hasCoveringIterator() const {
  ClusterEngine* ce = static_cast<ClusterEngine*>(EngineSelectorFeature::ENGINE);
  if (ce->isRocksDB()) {
    return _type == Index::TRI_IDX_TYPE_PRIMARY_INDEX ||
           _type == Index::TRI_IDX_TYPE_EDGE_INDEX ||
           _type == Index::TRI_IDX_TYPE_HASH_INDEX ||
           _type == Index::TRI_IDX_TYPE_SKIPLIST_INDEX ||
           _type == Index::TRI_IDX_TYPE_PERSISTENT_INDEX;
  }
  return false;
}

bool ClusterIndex::isSorted() const {
  ClusterEngine* ce = static_cast<ClusterEngine*>(EngineSelectorFeature::ENGINE);
  if (ce->isMMFiles()) {
    return  _type == Index::TRI_IDX_TYPE_SKIPLIST_INDEX ||
            _type == Index::TRI_IDX_TYPE_PERSISTENT_INDEX;
  } else if (ce->isRocksDB()) {
    return _type == Index::TRI_IDX_TYPE_PRIMARY_INDEX ||
    _type == Index::TRI_IDX_TYPE_EDGE_INDEX ||
    _type == Index::TRI_IDX_TYPE_HASH_INDEX ||
    _type == Index::TRI_IDX_TYPE_SKIPLIST_INDEX ||
    _type == Index::TRI_IDX_TYPE_PERSISTENT_INDEX ||
    _type == Index::TRI_IDX_TYPE_FULLTEXT_INDEX;
  } else {
    TRI_ASSERT(false);
    THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
  }
}

bool ClusterIndex::matchesDefinition(VPackSlice const& info) const {
  /*if (!Index::matchesDefinition(info)) {
    return false;
  }*/
  return Index::Compare(_info.slice(), info);
}

static std::vector<std::vector<arangodb::basics::AttributeName>> const
PrimaryIndexAttributes{{arangodb::basics::AttributeName("_id", false)},
  {arangodb::basics::AttributeName("_key", false)}};

bool ClusterIndex::supportsFilterCondition(arangodb::aql::AstNode const* node,
                                           arangodb::aql::Variable const* reference, size_t itemsInIndex,
                                           size_t& estimatedItems, double& estimatedCost) const {

  switch (_type) {
    case TRI_IDX_TYPE_PRIMARY_INDEX:{
      SimpleAttributeEqualityMatcher matcher(PrimaryIndexAttributes);
      return matcher.matchOne(this, node, reference, itemsInIndex, estimatedItems,
                              estimatedCost);
    }
    // should not be called for these
    case TRI_IDX_TYPE_GEO_INDEX:
    case TRI_IDX_TYPE_GEO1_INDEX:
    case TRI_IDX_TYPE_GEO2_INDEX:
    case TRI_IDX_TYPE_FULLTEXT_INDEX:
    case TRI_IDX_TYPE_IRESEARCH_LINK:
    case TRI_IDX_TYPE_NO_ACCESS_INDEX: {
      estimatedItems = itemsInIndex;
      estimatedCost = static_cast<double>(estimatedItems);
      return false;
    }
    case TRI_IDX_TYPE_HASH_INDEX:
    case TRI_IDX_TYPE_EDGE_INDEX:{
      
      break;
    }
    
    case TRI_IDX_TYPE_SKIPLIST_INDEX:
    case TRI_IDX_TYPE_PERSISTENT_INDEX:{
      
      break;
    }
    
    case TRI_IDX_TYPE_UNKNOWN:
      TRI_ASSERT(false);
      break;
  }
  return false;
}

bool ClusterIndex::supportsSortCondition(arangodb::aql::SortCondition const* sortCondition,
                                         arangodb::aql::Variable const* reference, size_t itemsInIndex,
                                         double& estimatedCost, size_t& coveredAttributes) const {
  
  switch (_type) {
    case TRI_IDX_TYPE_PRIMARY_INDEX:{
      
      
      
    }
      // should not be called for these
    case TRI_IDX_TYPE_GEO_INDEX:
    case TRI_IDX_TYPE_GEO1_INDEX:
    case TRI_IDX_TYPE_GEO2_INDEX:
    case TRI_IDX_TYPE_FULLTEXT_INDEX:
    case TRI_IDX_TYPE_IRESEARCH_LINK:
    case TRI_IDX_TYPE_NO_ACCESS_INDEX: {
      // by default, no sort conditions are supported
      coveredAttributes = 0;
      if (itemsInIndex > 0) {
        estimatedCost = itemsInIndex * std::log2(itemsInIndex);
      } else {
        estimatedCost = 0.0;
      }
      return false;
      return false;
    }
    case TRI_IDX_TYPE_HASH_INDEX:
    case TRI_IDX_TYPE_EDGE_INDEX:{
      
      break;
    }
      
    case TRI_IDX_TYPE_SKIPLIST_INDEX:
    case TRI_IDX_TYPE_PERSISTENT_INDEX:{
      
      break;
    }
      
    case TRI_IDX_TYPE_UNKNOWN:
      TRI_ASSERT(false);
      break;
  }
  return false;
  

}
