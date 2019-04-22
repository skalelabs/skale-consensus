# SKALE Consensus: a BFT Consensus engine in C++

[![Discord](https://img.shields.io/discord/534485763354787851.svg)](https://discord.gg/vvUtWJB)


SKALE consensus utilizes multiple block proposers.  Block proposers distribute proposals to nodes and  
collect a BLS-signature based data availability proofs. An Asynchronous Binary Byzantine Agreement is then
executed for each block proposal to reach consensus on whether it is data-available.  If multiple block proposals
are known to be data-available, a BLS-based common coin is used to select the winning proposal that is 
committed to the chain.

SKALE Consensus uses an Asynchronous Binary Byzantine Agreement (ABBA) protocol. The current implementation uses ABBA from Mostefaoui *et al.* In general, any ABBA protocol can be used so long as it has the following properties:

- Network model: protocol assumes asynchronous network messaging model.
- Byzantine nodes: protocol assumes less than 1/3 of nodes are Byzantine.
- Initial vote: protocol assumes each node makes an initial *yes* or *no* vote.
- Consensus vote: protocol terminates with consensus vote of either *yes* or *no*. Where consensus vote is *yes*, it is guaranteed that at least one honest node voted *yes*.

## An important note about production readiness:

The SKALE consensus is still in active development and contains bugs. This software should be regarded as _alpha software_. Development is still subject to competing the specification, security hardening, further testing, and breaking changes.  **This consensus engine has not yet been reviewed or audited for security.**

## Roadmap

_to be posted soon_

## Installation Requirements

SKALE consensus has been built and tested on Ubuntu.

Ensure that the required packages are installed by executing:

```
sudo apt-get update
sudo apt-get install -y cmake build-essential libgoogle-perftools-dev libboost-all-dev libgmp3-dev libssl-dev
```

### Building from source on Ubuntu (Development)

Configure the project build with the following commands.
```
cmake -H. -Bbuild                   # Configure the project and create a build directory.
cmake --build build -- -j$(nproc)   # Build all default targets using all cores.
```

### Running tests

Navigate to the testing directories and run `./consensusd .`

## Libraries
- [libBLS by SKALE Labs](https://skalelabs.com/)

## Contributing

**If you have any questions please ask our development community on [Discord](https://discord.gg/vvUtWJB).**

[![Discord](https://img.shields.io/discord/534485763354787851.svg)](https://discord.gg/vvUtWJB)

## License
![GitHub](https://img.shields.io/github/license/skalelabs/skale-consensus.svg)


Copyright (C) 2018-present SKALE Labs
