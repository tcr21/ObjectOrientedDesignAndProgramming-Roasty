#define MG_ENABLE_LOG 0
#include "RoastyServer.hpp"
#include "../Roasty.hpp"
#include "../Serialisation.hpp"
#include "../Storage/DiskStorage.hpp"
#include "../Storage/MemoryStorage.hpp"
#include "../Utilities.hpp"
#include <../lib/mongoose.c>
#include <algorithm>
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <mongoose.h>
#include <nlohmann/json.hpp>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>

class Dispatcher {
  mg_connection* c;

public:
  explicit Dispatcher(mg_connection* c) : c(c){};
  class URLComponent : public std::string {
  public:
    template <typename T> T get() {
      T result;
      std::stringstream(*this) >> result;
      return result;
    }
    friend std::istream& operator>>(std::istream& is, URLComponent& output) {
      std::getline(is, output, '/');
      return is;
    }
  };

  class Result {
    std::string content;
    std::string type;
    int const statusCode = 200;
    std::map<std::string, std::string> headers;

  public:
    Result(json const& j, int statusCode = 200, // NOLINT(hicpp-explicit-conversions)
           std::map<std::string, std::string> const& headers = {})
        : content(j.dump(2)), type("application/json"), statusCode(statusCode), headers(headers){};
    Result(std::string const& j) // NOLINT(hicpp-explicit-conversions)
        : content(j), type("text/html"){};
    auto getStatusCode() const { return statusCode; }
    auto const& getType() const { return type; }
    auto const& getContent() const { return content; }
    auto const& getHeaders() const { return headers; }
  };
  class RequestInformation : public std::vector<URLComponent> {
    std::string const acceptedType;
    std::string_view const body;

  public:
    using vector<URLComponent>::vector;
    template <typename InputIt>
    constexpr RequestInformation(InputIt first, InputIt last, std::string const& acceptedType,
                                 std::string_view const body)
        : vector(first, last), acceptedType(acceptedType), body(body){};
    std::string const& getAccept() { return acceptedType; }
    auto& getBody() const { return body; }
  };

private:
  std::map<std::string, std::map<std::string, std::function<Result(RequestInformation)>>> methods;

public:
  auto& getMethods() { return methods; }
  void reply(mg_http_message* hm) {
    for(auto& [url, function] : methods[std::string{hm->method.ptr, hm->method.len}]) {
      if(mg_http_match_uri(hm, url.c_str())) {
        auto in = std::istringstream(std::string(hm->uri.ptr, hm->uri.len));
        auto accept = std::string(mg_http_get_header(hm, "Accept")->ptr,
                                  mg_http_get_header(hm, "Accept")->len);

        try {
          auto result = function(RequestInformation(std::istream_iterator<URLComponent>(in),
                                                    std::istream_iterator<URLComponent>(), accept,
                                                    {hm->body.ptr, hm->body.len}));
          return mg_http_reply( // NOLINT
              c, result.getStatusCode(),
              (std::accumulate(result.getHeaders().begin(), result.getHeaders().end(),
                               "Content-Type: "s + result.getType(),
                               [](auto const& accumulator, auto& header) {
                                 return accumulator + header.first + ": " + header.second + "\n";
                               }) +
               "\r\n")
                  .c_str(), // NOLINT
              "%s", result.getContent().c_str());
        } catch(...) {
          return mg_http_reply(c, 500, "Content-Type: text/plain\r\n", // NOLINT
                               "not sure what happened");
        }
      }
    }
    mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "%s", "not a thing"); // NOLINT
  };
};

template <typename RoastyImplementation> void RoastyServer<RoastyImplementation>::startServer() {
  RoastyImplementation requestHandler;
  struct mg_mgr mgr = {};
  mg_mgr_init(&mgr);
  mg_http_listen(
      &mgr, "http://localhost:8000",
      [](mg_connection* c, int ev, void* ev_data, // NOLINT(bugprone-easily-swappable-parameters)
         void* fn_data) {                         // NOLINT(bugprone-easily-swappable-parameters)
        auto& requestHandler = *static_cast<RoastyImplementation*>(fn_data);
        if(ev == MG_EV_HTTP_MSG) {
          auto* hm = static_cast<struct mg_http_message*>(ev_data);
          auto d = Dispatcher(c);
          d.getMethods()["GET"]["/roasts"] = [&requestHandler](auto /*path*/) {
            return json(requestHandler.allRoasts());
          };
          d.getMethods()["POST"]["/roasts"] = [&requestHandler](auto request) {
            return Dispatcher::Result{
                201,
                json(201),
                {{"Location",
                  "/roasts/"s +
                      std::to_string(
                          requestHandler
                              .addRoast(Roast(json::parse(request.getBody()).at("timestamp")))
                              .getTimestamp())}}};
          };
          d.getMethods()["GET"]["/roasts/*"] = [&requestHandler](auto path) -> Dispatcher::Result {
            static auto indexFile = [] {
              auto input = std::ifstream(STRINGIZE(ROASTY_SOURCE_DIR) "/www/addRoast.html");
              return std::string{std::istreambuf_iterator<char>(input),
                                 std::istreambuf_iterator<char>()};
            }();
            return (path.getAccept().find("text/html") == 0)
                       ? (Dispatcher::Result)indexFile
                       : json(requestHandler.getRoast(path[2].template get<long>()));
          };
          d.getMethods()["GET"]["/beans"] = [&requestHandler](auto /*unused*/) {
            return json(requestHandler.allBeans());
          };
          d.getMethods()["GET"]["/roasts/*/events"] = [&requestHandler](auto request /*unused*/) {
            auto& roast = requestHandler.getRoast(request[2].template get<long>());
            auto result = RangeGenerator<Event const>(
                [&roast](auto i) -> auto const& { return roast.getEvent(i); }, roast.getEventCount());
            return json(result);
          };
          d.getMethods()["POST"]["/roasts/*/events"] = [&requestHandler](auto request /*unused*/) {
            auto body = json::parse(request.getBody());
            auto& roast = requestHandler.getRoast(request[2].template get<long>());
            roast.addEvent(Event(body.at("type"), body.at("timestamp").template get<long>(),
                                 body.count("value")
                                     ? new EventValue(body.at("value").template get<long>())
                                     : nullptr));
            return json("okay");
          };
          d.getMethods()["PUT"]["/roasts/*/events/*"] = [&requestHandler](auto request /*unused*/) {
            auto& event = requestHandler.getRoast(request[2].template get<long>())
                              .getEventByTimestamp(request[4].template get<long>());
            event.getValue()->setValue(
                json::parse(request.getBody()).at("value").template get<long>());
            return json("okay");
          };

          d.getMethods()["DELETE"]["/roasts/*/events/*"] =
              [&requestHandler](auto request /*unused*/) {
                requestHandler.getRoast(request[2].template get<long>())
                    .removeEventByTimestamp(request[4].template get<long>());
                return json("okay");
              };

          d.getMethods()["POST"]["/roasts/*/blends"] = [&requestHandler](auto request) {
            auto body = json::parse(request.getBody());
            requestHandler.addIngredientToRoast(
                request[2].template get<long>(),
                Ingredient(
                    requestHandler.getBeanByName(body.at("name").template get<std::string>()),
                    body.at("amount").template get<long>()));

            return json("okay");
          };
          d.getMethods()["PATCH"]["/roasts/*/blends/*"] =
              d.getMethods()["PUT"]["/roasts/*/blends/*"] =
                  [&requestHandler](auto request /*unused*/) {
                    auto& roast = requestHandler.getRoast(request[2].template get<long>());
                    for(auto i = 0U; i < roast.getIngredientsCount(); i++) {
                      if(roast.getIngredient(i).getBean().getName() ==
                         request[4].template get<std::string>()) {
                        roast.getIngredient(i).setAmount(
                            json::parse(request.getBody()).at("amount").template get<long>());
                      }
                    }
                    return json("okay");
                  };
          d.getMethods()["DELETE"]["/roasts/*/blends/*"] =
              [&requestHandler](auto request /*unused*/) {
                requestHandler.getRoast(request[2].template get<long>())
                    .removeIngredientByBeanName(request[4].template get<std::string>());
                return json("okay");
              };
          d.getMethods()["DELETE"]["/roasts/*"] = [&requestHandler](auto request /*unused*/) {
            requestHandler.deleteRoast(request[2].template get<long>());
            return json("okay");
          };

          d.getMethods()["POST"]["/beans"] = [&requestHandler](auto request /*unused*/) {
            requestHandler.addBean(Bean{json::parse(request.getBody()).at("name")});
            return json("okay");
          };
          d.getMethods()["GET"]["/"] = [](auto /*unused*/) {
            static auto indexFile = [] {
              auto input = std::ifstream(STRINGIZE(ROASTY_SOURCE_DIR) "/www/index.html");
              return std::string{std::istreambuf_iterator<char>(input),
                                 std::istreambuf_iterator<char>()};
            }();
            return indexFile;
          };
          d.reply(hm);
        }
      },
      &requestHandler);

  static auto done = false;
  signal(SIGINT, [](auto) { done = true; });
  std::cout << "open http://localhost:8000 to access the application" << std::endl;
  std::cout << "press ctrl+c to quit" << std::endl;
  while(!done) { // NOLINT(bugprone-infinite-loop)
    mg_mgr_poll(&mgr, 1000);
  }
  std::cout << "shutting down server" << std::endl;
  mg_mgr_free(&mgr);

  std::cout << "flushing to disk" << std::endl;
  requestHandler.commit();
}

template class RoastyServer<Roasty<MemoryStorage>>;
template class RoastyServer<Roasty<DiskStorage>>;
