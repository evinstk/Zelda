#include "goal_think.h"

#include <cassert>

namespace te
{
	GoalThink::GoalThink(ZeldaEntity& owner)
		: mOwner(owner)
	{}

	void GoalThink::arbitrate()
	{
		float best = 0;
		GoalEvaluator<ZeldaEntity>* mostDesirable = NULL;

		for (auto& pEvaluator : mEvaluators)
		{
			float desirability = pEvaluator->calculateDesirability(mOwner);
			if (desirability >= best)
			{
				best = desirability;
				mostDesirable = pEvaluator.get();
			}
		}

		assert(mostDesirable != NULL);
		mostDesirable->setGoal(mOwner);
	}

	void GoalThink::activate()
	{
		setStatus(Status::ACTIVE);
	}

	Goal<ZeldaEntity>::Status GoalThink::process()
	{
		return processSubgoals();
	}

	void GoalThink::terminate() {}
}
