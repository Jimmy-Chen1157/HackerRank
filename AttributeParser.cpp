#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>




struct Attr
{
    std::string key;
    std::string value;
};

struct TagAttr
{
    std::string tag;
    std::vector<Attr> attr;
};

enum class Mode
{
    key = 0,
    value = 1,
    NONE = -1
};

class Query {

private:
    std::vector<std::string> query;
    std::string val;

public:
    Query(const std::string& line)
        : val("")
    {
        FormQuery(line);
    }

    Query() {}

    std::vector<std::string> GetQuery() const { return query; }
    std::string GetVal() const { return val; }

private:

    void FormQuery(const std::string& line)
    {
        int index = 0;
        std::string curr_query = "";
        Mode mode = Mode::NONE;

        while (index < line.size()) {

            if (line[index] == '~')
            {
                mode = Mode::value;
            }

            if (mode == Mode::value && line[index] != '~')
            {
                val += line[index];
            }


            if (line[index] == '.' || line[index] == '~')
            {
                query.push_back(curr_query);
                curr_query = "";
            }
            else if (mode != Mode::value) {
                curr_query += line[index];
            }

            index++;
        }

    }
};

class Queries : public Query{

private:
    std::vector<Query> m_queries;

public:
    Queries() {}

    std::vector<Query> GetQueries() const { return m_queries; }

    void AddQueries(const std::string& line)
    {
        m_queries.emplace_back(Query(line));
    }
};


class Tag {

public:
    std::vector<TagAttr> tagArr;

public:
 
    Tag() {}

    std::vector<TagAttr> GetTags() const { return tagArr; }

    void ParseTag(const std::string& line)
    {
        int index = 1;
        while (index < line.size())
        {
            if (line[index] == '/')
            {
                return;
            }

            std::string curr_tag = "";
            while (line[index] != ' ')
            {
                curr_tag += line[index];
                index++;
            }

            std::vector<Attr> attr;
            Mode mode = Mode::NONE;
            Mode prevMode = Mode::NONE;
            bool open = false;

            std::string curr_key = "", curr_value = "";

            while (true)
            {

                if (curr_key != "" && curr_value != "" && (prevMode != mode || line[index] == '>'))
                {

                    attr.push_back({ curr_key, curr_value });

                    curr_key = "";
                    curr_value = "";

                }

                prevMode = mode;

                if (line[index] == '>') {
                    tagArr.push_back({ curr_tag, attr });
                    return;
                }


                if (line[index] == '"')
                {
                    mode = Mode::value;
                    open = !open;
                }
                else if (line[index] == ' ' && !open)
                {
                    mode = Mode::key;
                }

                if (mode == Mode::value && line[index] != '"')
                {
                    curr_value += line[index];
                }

                if (mode == Mode::key && line[index] != '=' && line[index] != ' ')
                {
                    curr_key += line[index];
                }

                index++;
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Tag& tag);
};

std::ostream& operator<<(std::ostream& os, const Tag& tag)
{
    for (auto tags : tag.tagArr)
    {
        os << tags.tag << "---" << std::endl;
        for (auto att : tags.attr)
        {
            os << "{ " << att.key << " : " << att.value << " }" << std::endl;
        }
    }
    return os;
}

class FileStream
{
private:
    std::string m_filepath;
    std::string m_hrml, m_queries;

public:
    FileStream(const std::string& filepath)
        : m_filepath(filepath), m_hrml(""), m_queries("")
    {}

    void GetStream(Tag& Tags, Queries& Queries)
    {
        std::ifstream stream(m_filepath);
        std::string line;
        
        getline(stream, line);
        for (int index=0; index < line.size(); index++)
        {
            if (line[index] != ' ')
            {
                if (m_hrml != "")
                {
                    m_queries += line[index];
                }
                else
                {
                    m_hrml += line[index];
                }
            }
        }
        
        int hrml = std::stoi(m_hrml);
        int queries = std::stoi(m_queries);

        while (hrml--)
        {
            getline(stream, line);
            Tags.ParseTag(line);
        }

        while (queries--)
        {
            getline(stream, line);
            Queries.AddQueries(line);
        }

    }

    void FindQuery(const Tag& tags, const Queries& queries)
    {
        for (auto query : queries.GetQueries()) {
            int index = -1;
            for (auto tag : query.GetQuery())
            {
                index++;
                if (tag != tags.tagArr[index].tag)
                {
                    std::cout << "Not Found!" << std::endl;
                    break;
                }
            }

            
            std::vector<Attr>::const_iterator it = tags.tagArr[index].attr.begin();
            while (it != tags.tagArr[index].attr.end())
            {
                if (it->key == query.GetVal())
                {
                    std::cout << it->value << std::endl;
                    break;
                }
                it++;
            }

            if (it == tags.tagArr[index].attr.end())
            {
                std::cout << "Not Found!" << std::endl;
            }

        }
    }

};

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    
    std::string filepath = "res/HRMLParsing.hrml";

    Tag tags;
    Queries queries;

    FileStream fs(filepath);
    fs.GetStream(tags, queries);
    fs.FindQuery(tags, queries);

    return 0;
}

