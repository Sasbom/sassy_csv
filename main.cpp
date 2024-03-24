#include <iostream>
#include <string_view>
#include <vector>
#include <variant>
#include <cmath>

void trim_string(std::string& str){
    auto start = str.find_first_not_of(' ');
    auto end = str.find_last_not_of(' ');
    str = str.substr(start,end+1);
}

bool is_char_number(char& c){
    return ((c >= 48) && (c <= 57));
}

int char_to_num(char& c){
    if (is_char_number(c))
        return static_cast<int>(c - 48);
    else
        return 0;
}

bool is_char_number_ignore(char& c){
    return ((c == '\"') || (c == ' '));
}

bool is_char_alphabetic(char& c){
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

using csv_var_t = std::variant<std::string,int,double>;

enum csv_type_enum {
    CSV_STRING,
    CSV_INT,
    CSV_FLOAT
};

struct CSVProcessor {
    std::vector<std::string> header{};
    std::vector<std::vector<csv_var_t>> rows{};
    std::vector<csv_type_enum> inferred_types{};

    struct CSVDataHelper{
        bool open_quote = false;
        bool had_quote = false;
        bool is_number = true;
        bool has_dot = false;
        bool number_started = false;
        bool number_finished = false;
        bool sign = true; //true +, false -
        double cur_double{};
        double dot_count{1};

        void reset_number(){
            is_number = true;
            has_dot = false;
            number_started = false;
            number_finished = false;
            sign = true; //true +, false -
            cur_double = 0;
            dot_count = 1;
        }
    };

    void process_csv(std::string const & str) {
        bool read_header = true;
        
        auto data = CSVDataHelper();
        
        bool& open_quote = data.open_quote;
        bool& had_quote = data.had_quote;

        bool& is_number = data.is_number;
        bool& has_dot = data.has_dot;
        bool& number_started = data.number_started;
        bool& number_finished = data.number_finished;
        bool& sign = data.sign; //true +, false -
        std::vector<csv_var_t> row{};
        std::string current{};
        double& cur_double = data.cur_double;
        double& dot_count = data.dot_count;

        for(auto it = str.begin(); it != str.end(); it++){
            char c = *it;

            if (!read_header)
            {

            if (is_char_alphabetic(c) && is_number){
                is_number = false;
            }

            // number parsing (keep string parsing going,)

            if (c == '-' && is_number && !number_started){
                sign = !sign;
            }

            // dot checking
            if (c == '.' && !has_dot && is_number){
                has_dot = true;
            } else if (c == '.' && has_dot && is_number){
                is_number = false;
                number_finished = true;
            }

            if (is_char_number(c) && is_number){
                if (!number_started)
                    number_started = true;

                if (!has_dot){
                    cur_double *= 10;
                    cur_double += char_to_num(c);
                } else {
                    double char_num = char_to_num(c) * std::pow(10,-dot_count);
                    cur_double += char_num;
                    dot_count++;
                }

            } else if (is_char_number_ignore(c) && is_number && number_started && !number_finished){
                number_finished = true;
            } else if (is_char_number(c) && is_number && number_finished){
                is_number = false;
            }
            }

            if (c == '\"' && !open_quote){
                open_quote = true;
                had_quote = true;
                continue;
            }

            if (c == '\"' && open_quote){
                // handle double qoutes ""
                char next = *(it + 1);
                if (next == '\"'){
                    current += c;
                    it++;
                    continue;
                }
                open_quote = false;
                continue;
            }

            if (c == ',' && !open_quote){
                if (read_header){
                    if (!had_quote){
                        trim_string(current);
                    }
                    had_quote = false;
                    header.push_back(current);
                    current.clear();
                    continue;
                } else {
                    if (is_number && !sign){
                        cur_double *= -1;
                    }

                    if (is_number && !has_dot){
                        row.push_back(static_cast<int>(cur_double));
                        current.clear();
                        // reset number stuff
                        data.reset_number();
                        continue;
                    } else if (is_number && has_dot) {
                        row.push_back(cur_double);
                        current.clear();
                        // reset number stuff
                        data.reset_number();
                        continue;
                    }

                    // reset number stuff
                    data.reset_number();
            
                    if (!had_quote){
                        trim_string(current);
                    }
                    had_quote = false;
                    row.push_back(current);
                    current.clear();
                    continue;
                }
            }

            if (c == '\n'){
                if (read_header){
                    read_header = false;
                    if (!had_quote){
                        trim_string(current);
                    }
                    had_quote = false;
                    header.push_back(current);
                    current.clear();
                    continue;
                } else {
                    if (is_number && !sign){
                        cur_double *= -1;
                    }

                    if (is_number && !has_dot){
                        row.push_back(static_cast<int>(cur_double));
                        rows.push_back(row);
                        row.clear();
                        current.clear();
                        // reset number stuff
                        data.reset_number();
                        continue;
                    } else if (is_number && has_dot) {
                        row.push_back(cur_double);
                        rows.push_back(row);
                        row.clear();
                        current.clear();
                        // reset number stuff
                        data.reset_number();
                        continue;
                    }

                    // reset number stuff
                    data.reset_number();

                    if (!had_quote){
                        trim_string(current);
                        had_quote = false;
                    }
                    row.push_back(current);
                    rows.push_back(row);
                    row.clear();
                    current.clear();
                    //current += "\n"; // temp newline for display of data
                    continue;
                }
            }

            current += c;

        }
    }

    void print_rows(){
        for (auto& h: header){
            std::cout << h << " | ";
        }

        std::cout << "\n";

        for(auto& row: rows){
            for(auto& el: row){
                std::size_t idx = el.index();
                
                switch(idx){
                    case 0:
                        std::cout << std::get<std::string>(el) << " , ";
                        break;
                    case 1:
                        std::cout << std::get<int>(el) << " , ";
                        break;
                    case 2:
                        std::cout << std::get<double>(el) << " , ";
                        break;
                }
            }

            std::cout << "\n";
        }
    }

};

// legacy
void process_csv(std::string const & str){
    bool read_header = true;
    bool open_quote = false;
    bool had_quote = false;

    bool is_number = true;
    bool has_dot = false;
    bool number_started = false;
    bool number_finished = false;
    bool sign = true; //true +, false -
    std::vector<std::string> header{};
    std::vector<std::string> items{};
    std::string current{};
    double cur_double{};
    double dot_count{1};

    for(auto it = str.begin(); it != str.end(); it++){
        char c = *it;

        if (!read_header)
        {

        if (is_char_alphabetic(c) && is_number){
            is_number = false;
        }

        // number parsing (keep string parsing going,)

        if (c == '-' && is_number && !number_started){
            sign = !sign;
        }

        // dot checking
        if (c == '.' && !has_dot && is_number){
            has_dot = true;
        } else if (c == '.' && has_dot && is_number){
            is_number = false;
            number_finished = true;
        }

        if (is_char_number(c) && is_number){
            if (!number_started)
                number_started = true;
            
            if (!has_dot){
                cur_double *= 10;
                cur_double += char_to_num(c);
            } else {
                double char_num = char_to_num(c) * std::pow(10,-dot_count);
                cur_double += char_num;
                dot_count++;
            }

        } else if (is_char_number_ignore(c) && is_number && number_started && !number_finished){
            number_finished = true;
        } else if (is_char_number(c) && is_number && number_finished){
            is_number = false;
        }
        }

        if (c == '\"' && !open_quote){
            open_quote = true;
            had_quote = true;
            continue;
        }

        if (c == '\"' && open_quote){
            // handle double qoutes ""
            char next = *(it + 1);
            if (next == '\"'){
                current += c;
                it++;
                continue;
            }
            open_quote = false;
            continue;
        }

        if (c == ',' && !open_quote){
            if (read_header){
                if (!had_quote){
                    trim_string(current);
                }
                had_quote = false;
                header.push_back(current);
                current.clear();
                continue;
            } else {
                if (is_number && !sign){
                    cur_double *= -1;
                }

                if (is_number)
                    std::cout << "number parsed: " << cur_double << "\n";
                // reset number stuff
                is_number = true;
                has_dot = false;
                number_started = false;
                number_finished = false;
                sign = true; //true +, false -
                cur_double = 0;
                dot_count = 1;

                if (!had_quote){
                    trim_string(current);
                }
                had_quote = false;
                items.push_back(current);
                current.clear();
                continue;
            }
        }

        if (c == '\n'){
            if (read_header){
                read_header = false;
                if (!had_quote){
                    trim_string(current);
                }
                had_quote = false;
                header.push_back(current);
                current.clear();
                continue;
            } else {
                if (is_number && !sign){
                    cur_double *= -1;
                }

                if (is_number)
                    std::cout << "number parsed: " << cur_double << "\n";

                // reset number stuff
                is_number = true;
                has_dot = false;
                number_started = false;
                number_finished = false;
                sign = true; //true +, false -
                cur_double = 0;
                dot_count = 1;

                if (!had_quote){
                    trim_string(current);
                    had_quote = false;
                }
                items.push_back(current);
                current.clear();
                current += "\n"; // temp newline for display of data
                continue;
            }
        }

        current += c;
        
    }

    std::cout << "header:\n";

    for(auto& el : header){
        std::cout << el << "|";
    }

    std::cout << "\ndata:\n";

    for(auto& el : items){
        std::cout << el << "|";
    }

}

int main(){

    std::string csv_str = "\"field1\",\"field2\",\"field3\",\"field4\",\"field5   \",\"field6\"\n"
                          "1,  1.5  ,127.0.0.1,   string1,\"string 2\",\"string \"\"3\"\", if you can call it that\"\n"
                          "-2,5.75,192.168.1.420,\"67.45786\",\"   string 5\",\"string \"\"6\"\", if I can call it that\"\n";

//    std::string csv_str_2 = R"("field1","field2","field3","field4","field5   ","field6"
//1,  1.5 ,127.0.0.1,string1,"string 2","string ""3"", if you can call it that"
//-2,5.75,192.168.1.420,string 4  ,"   string 5","string ""6"", if I can call it that"
//)";

//    process_csv(csv_str);
    auto processor = CSVProcessor();
    processor.process_csv(csv_str);
    processor.print_rows();
}