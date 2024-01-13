#pragma once

#include <memory>
#include <functional>
#include <sigc++/signal.h>
#include "inode.h"

namespace scene
{

namespace merge
{

enum class ActionType
{
    NoAction,
    AddEntity,
    RemoveEntity,
    AddKeyValue,
    RemoveKeyValue,
    ChangeKeyValue,
    AddChildNode,
    RemoveChildNode,
    ConflictResolution,
};

enum class ConflictType
{
    // Not a conflict
    NoConflict,

    // Entity has been removed in target, source tries to modify it
    ModificationOfRemovedEntity,

    // Entity has been modified in target, source tries to remove it
    RemovalOfModifiedEntity,

    // Key Value has been removed in targed, source tries to change it
    ModificationOfRemovedKeyValue,

    // Key Value has been modified in targed, source tries to remove it
    RemovalOfModifiedKeyValue,

    // Both sides try to set the same key to a different value
    SettingKeyToDifferentValue,
};

enum class ResolutionType
{
    Unresolved,
    RejectSourceChange,
    ApplySourceChange,
};

/**
 * Represents a merge action, i.e. one single step of a merge operation.
 * Only active actions will be processed when the merge run starts.
 */ 
class IMergeAction
{
public:
    virtual ~IMergeAction() {}

    using Ptr = std::shared_ptr<IMergeAction>;

    // The type performed by this action
    virtual ActionType getType() const = 0;

    // Activate this action, it will be executed during the merge
    virtual void activate() = 0;

    // Deactivate this action, it will NOT be executed during the merge
    virtual void deactivate() = 0;

    // Returns the active state of this action
    virtual bool isActive() const = 0;

    // Applies all changes defined by this action (if it is active,
    // deactivated action will not take any effect).
    // It's the caller's responsibility to set up any Undo operations.
    // Implementations are allowed to throw std::runtime_errors on failure.
    virtual void applyChanges() = 0;

    // Returns the node this action is affecting when applied
    // This is used to identify the scene node and display it appropriately
    virtual scene::INodePtr getAffectedNode() = 0;
};

class IEntityKeyValueMergeAction :
    public virtual IMergeAction
{
public:
    virtual ~IEntityKeyValueMergeAction() {}

    using Ptr = std::shared_ptr<IEntityKeyValueMergeAction>;

    // Gets the key name affected by this action
    virtual const std::string& getKey() const = 0;

    // Gets the value that is going to be set by this action
    virtual const std::string& getValue() const = 0;

    // The action is usually applying its value as soon as it is inserted into
    // the scene for preview. It remembers the original entity key value,
    // use tis method to retrieve it.
    virtual const std::string& getUnchangedValue() const = 0;
};

class IConflictResolutionAction :
    public virtual IMergeAction
{
public:
    virtual ~IConflictResolutionAction() {}

    using Ptr = std::shared_ptr<IConflictResolutionAction>;

    // The exact conflict type of this node
    virtual ConflictType getConflictType() const = 0;

    // Gets the value that is going to be set by this action
    virtual const IMergeAction::Ptr& getSourceAction() const = 0;

    // The action that happened in the target (can be empty)
    virtual const IMergeAction::Ptr& getTargetAction() const = 0;

    // The source entity node causing the conflict
    virtual const INodePtr& getConflictingSourceEntity() const = 0;

    // The affected entity node in the target map
    virtual const INodePtr& getConflictingTargetEntity() const = 0;

    // Whether this action has been resolved at all, and what has been chosen
    virtual ResolutionType getResolution() const = 0;

    // Resolve this action by either accepting or rejecting the source change
    virtual void setResolution(ResolutionType resolution) = 0;
};

// A MergeOperation groups one or more merge actions
// together in order to apply a set of changes from source => target
class IMergeOperation
{
public:
    using Ptr = std::shared_ptr<IMergeOperation>;

    virtual ~IMergeOperation() {}

    // Returns the name/path of the source scene (or a string resembling it)
    virtual std::string getSourcePath() = 0;

    // Returns the name/path of the base scene (or a string resembling it)
    // Will return an empty string if there is no base scene defined
    virtual std::string getBasePath() = 0;

    // Executes all active actions defined in this operation
    virtual void applyActions() = 0;

    // Whether this operation has any actions to perform
    virtual bool hasActions() = 0;

    // Adds a new action to this operation
    virtual void addAction(const IMergeAction::Ptr& action) = 0;

    // Invokes the given functor for each action in this operation
    virtual void foreachAction(const std::function<void(const IMergeAction::Ptr&)>& visitor) = 0;

    // Enables or disable merging of selection groups
    virtual void setMergeSelectionGroups(bool enabled) = 0;

    // Enables or disable merging of layers
    virtual void setMergeLayers(bool enabled) = 0;

    // Signal which is emitted when an action is added to this operation
    virtual sigc::signal<void(const IMergeAction::Ptr&)>& sig_ActionAdded() = 0;
};

}

/**
 * Special scene node type representing a change conducted by a merge action, 
 * i.e. addition, removal or changing a node in the scene.
 */
class IMergeActionNode :
    public virtual scene::INode
{
public:
    virtual ~IMergeActionNode() {}

    // Return the action type represented by this node
    virtual merge::ActionType getActionType() const = 0;

    // Return the node this action is affecting
    virtual scene::INodePtr getAffectedNode() = 0;

    // The number of merge actions associated to this node.
    // This can be 0 if the node has been cleared out after completing a merge operation
    virtual std::size_t getMergeActionCount() = 0;

    // Returns true if this node has one or more active actions.
    // If all associated actions have been deactivated, this returns false.
    virtual bool hasActiveActions() = 0;

    // Iterate over all actions of this node
    virtual void foreachMergeAction(const std::function<void(const merge::IMergeAction::Ptr&)>& functor) = 0;
};

}
