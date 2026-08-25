# Audit Brief for the Planning Packet

The next agent should audit this folder and the applied GitHub Project before any
further broad remote changes or any issue is treated as Ready.

## Required checks

- Confirm the existing five project docs are the authoritative behavioral
  source and that no backlog item invents a conflicting product requirement.
- Confirm the current scaffold and working-tree caveat are accurately reflected.
- Check that every child issue has one primary observable outcome, a bounded
  scope, acceptance criteria, a verification path, dependencies, and stop
  conditions.
- Check that no issue prescribes a class name, method signature, exact file
  decomposition, or hidden implementation strategy unless the requirement is
  genuinely externally observable.
- Check that issue dependencies are acyclic and that the proposed critical path
  is valid.
- Check that the two-hours-per-week constraint is respected: no issue should
  require an unbounded phase, and no sprint should depend on completing every
  listed issue.
- Check that all open behavior, compatibility, toolchain, and testing decisions
  are visible in the decision ledger.
- Check that `Ready` is reserved for dependency-free, decision-complete work.
- Check that the local packet and the applied GitHub Project do not diverge.

## Questions for the auditor

1. Is BRK-001 genuinely the safest first handoff given the uncommitted
   scaffold?
2. Are any issues still too large for one focused coding session?
3. Are any requirements accidentally implementation instructions?
4. Are there missing verification seams for persistence, audio, or screen flow?
5. Should any sprint be reordered because the learner would gain feedback
   sooner without increasing architectural churn?

## Audit outcome format

Return one of `PASS`, `REVISE`, or `BLOCKED`, followed by concrete issue keys and
recommended changes. Do not create remote entries during the audit.
