#include "common.h"
#include "utils.h"
#include "customer.h"
#include "clerk.h"

#include <thread>

Customer::Customer(int id_) : id(id_) {}

void Customer::operator()(int numCones) {
  atomicCout(cout_mx, "Customer ", id, " is buying ", numCones, " cones ...");

  Semaphore clerkDone;

  std::vector<ThreadRAII> clerk_threads;
  for (int i = 0; i < numCones; ++i) {
    clerk_threads.emplace_back(ThreadRAII(std::thread(Clerk(i), std::ref(clerkDone)),
                                          ThreadRAII::DtorAction::join));
  }

  for (int i = 0; i < numCones; ++i) {
    clerkDone.wait();
  }

  atomicCout(cout_mx, "Customer ", id, " walking to cashier...");

  line.lock.wait();
  int place = line.number++;
  line.lock.notify();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  line.requested.notify();
  line.customer[place]->wait();

  atomicCout(cout_mx, "Customer ", id, " 's shopping DONE!");

}
