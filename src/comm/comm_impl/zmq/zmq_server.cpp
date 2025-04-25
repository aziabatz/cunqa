
namespace cunqa {
namespace comm {

struct Impl {

}

Server(const NetConfig& net_config) :
    comm_strat{std::make_unique<SelectedServer>(net_config)} 
{ }

inline std::string recv_circuit() { return comm_strat->recv_data(); }

inline void accept() { comm_strat->accept(); }

inline void send_result(const std::string& result) { 
    try {
        comm_strat->send_result(result);
    } catch (const std::exception& e) {
        throw ServerException(e.what());
    }
}

inline void close() {comm_strat->close(); }

} // End of comm namespace
} // End of cunqa namespace
