#include "goal_think.h"
#include "goal_evaluator_move_to_position.h"

#include <cassert>

namespace te
{
	GoalThink::GoalThink(ZeldaEntity& owner)
		: mOwner(owner)
		, mEvaluators()
	{
		mEvaluators.push_back(std::make_unique<GoalEvaluator_MoveToPosition>());
		assert(mEvaluators.size() != 0);
	}

	void GoalThink::arbitrate()
	{
		float best = 0;
		GoalEvaluator<ZeldaEntity>* mostDesirable = nullptr;

		for (auto& pEvaluator : mEvaluators)
		{
			float desirability = pEvaluator->calculateDesirability(mOwner);
			if (desirability >= best)
			{
				best = desirability;
				mostDesirable = pEvaluator.get();
			}
		}

		assert(mostDesirable != nullptr);
		mostDesirable->setGoal(mOwner);
	}

	void GoalThink::activate()
	{
		setStatus(Status::ACTIVE);
	}

	Goal<ZeldaEntity>::Status GoalThink::process(const sf::Time& dt)
	{
		return processSubgoals(dt);
	}

	void GoalThink::terminate() {}
}
