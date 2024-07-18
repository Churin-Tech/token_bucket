# token_bucket
Thread-safe (atomic) token bucket primitive(https://en.wikipedia.org/wiki/Token_bucket). This TokenBucket is a minimal implementation of the folly token bucket.

# Usgae
## Download
enter your project include dir.  
```
git clone git@github.com:Churin-Tech/token_bucket.git
```
or use Cmake `Declare`.

---

## Use
```
#include <thread>
#include <iostream>

#include "token_bucket.h"

using namespace TB;

int main(int argc, char *argv[]) {
    TokenBucket tb(100);
    auto tb_ptr = std::make_shared<TokenBucket>(100);
    for (size_t idx; idx < 10; idx++) {
        std::thread t([&idx, &tb_ptr](){
            while (1) {
                if (!tb_ptr->Consume(1)) {
                    std::cout << "idx: " << idx << " Consume failed" << std::endl;
                }
            }
        });
    }
}
```

## Performance
