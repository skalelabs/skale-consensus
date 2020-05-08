FROM skalenetwork/consensust_base:latest

COPY . /consensust
WORKDIR /consensust

ENV CC gcc-7
ENV CXX g++-7
ENV TARGET all

RUN deps/build.sh

RUN cmake . -Bbuild -DCMAKE_BUILD_TYPE=Debug  -DCOVERAGE=ON -DMICROPROFILE_ENABLED=0
RUN bash -c "cmake --build build -- -j$(nproc)"

ENTRYPOINT ["/consensust/scripts/start.sh"]

