#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class DBConnection_Interface{
  protected:
    std::string ip_;
  public:
    DBConnection_Interface(const std::string& ip): ip_(ip){}
    virtual ~DBConnection_Interface(){}
    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual void execQuery(const std::string& query) = 0;
};

class DBConnection : public DBConnection_Interface{
  public:
    DBConnection(const std::string& ip): DBConnection_Interface(ip){}
    ~DBConnection(){}
    virtual bool open() override
    {
      if (ip_.empty()){
        throw std::invalid_argument("IP is empty");
      }
      if (ip_ == "0.0.0.0"){
        return false;
      }
      return true;
    }
    virtual bool close() override
    {
      if (ip_.empty()){
        throw std::invalid_argument("IP is empty");
      }
      if (ip_ == "0.0.0.0"){
        return false;
      }
      return true;
    }
    virtual void execQuery(const std::string& query) override
    {
      if (query.empty()){
        throw std::invalid_argument("query is empty");
      }
      std::cout << "Query: " << query << " has done\n";
    }
};


//=============================================================================
//Тесты класса DBConnection
//Тест корректного результата методов
class DBConnectionTestCorrect : public ::testing::Test{
  protected:
    virtual ~DBConnectionTestCorrect(){}
    virtual void SetUp() override
    {
      connection = new DBConnection("127.0.0.1");
    }
    virtual void TearDown() override
    {
      delete connection;
    }
    DBConnection* connection;
};

//Тест некорректного результата методов
class DBConnectionTestIncorrect : public ::testing::Test{
  protected:
    virtual ~DBConnectionTestIncorrect(){}
    virtual void SetUp() override
    {
      connection = new DBConnection("0.0.0.0");
    }
    virtual void TearDown() override
    {
      delete connection;
    }
    DBConnection* connection;
};

//Тест выбросов исключений из методов
class DBConnectionTestExceptions : public ::testing::Test{
  protected:
    virtual ~DBConnectionTestExceptions(){}
    virtual void SetUp() override
    {
      connection = new DBConnection("");
    }
    virtual void TearDown() override
    {
      delete connection;
    }
    DBConnection* connection;
};

TEST_F(DBConnectionTestCorrect, correct)
{
	EXPECT_EQ(connection->open(), true);
  EXPECT_EQ(connection->close(), true);
}

TEST_F(DBConnectionTestIncorrect, incorrect)
{
	EXPECT_EQ(connection->open(), false);
  EXPECT_EQ(connection->close(), false);
}

TEST_F(DBConnectionTestExceptions, throw_exception)
{
  EXPECT_THROW(connection->open(), std::invalid_argument);
  EXPECT_THROW(connection->execQuery(""), std::invalid_argument);
  EXPECT_THROW(connection->close(), std::invalid_argument);
}



//=============================================================================
class ClassThatUseDB{
  private:
    DBConnection_Interface* dbconnection_;
  public:
    ClassThatUseDB(DBConnection_Interface* dbconnection):
      dbconnection_(dbconnection){};
    ~ClassThatUseDB(){};

    void openConnection()
    {
      dbconnection_->open();
    }
    void useConnection(const std::string& query)
    {
      dbconnection_->execQuery(query);
    }
    void closeConnection()
    {
      dbconnection_->close();
    }
};




class DBConnection_Mock : public DBConnection_Interface{
  public:
    DBConnection_Mock(const std::string& ip): DBConnection_Interface(ip){}
    MOCK_METHOD(bool, open, (), (override));
    MOCK_METHOD(bool, close, (), (override));
    MOCK_METHOD(void, execQuery, (const std::string& query), (override));
};


//Тесты взаимодействия классов DBConnection_Interface и ClassThatUseDB
//=============================================================================
TEST(ClassThatUseDB, openConnection_ok)
{
  const std::string correctIP = "127.0.0.1";
  DBConnection_Mock mockConnection(correctIP);
  ClassThatUseDB userDB(&mockConnection);

  //Будет вызван метод open() который вернёт true
  EXPECT_CALL(mockConnection, open())
    .Times(1)
    .WillOnce(::testing::Return(true));

  userDB.openConnection();
}

TEST(ClassThatUseDB, openConnection_not_ok)
{
  const std::string incorrectIP = "0.0.0.0";
  DBConnection_Mock mockConnection(incorrectIP);
  ClassThatUseDB userDB(&mockConnection);

  EXPECT_CALL(mockConnection, open())
    .Times(1)
    .WillOnce(::testing::Return(false));

  userDB.openConnection();
}

TEST(ClassThatUseDB, closeConnection_ok)
{
  const std::string correctIP = "127.0.0.1";
  DBConnection_Mock mockConnection(correctIP);
  ClassThatUseDB userDB(&mockConnection);

  EXPECT_CALL(mockConnection, close())
    .Times(1)
    .WillOnce(::testing::Return(true));

  userDB.closeConnection();
}

TEST(ClassThatUseDB, closeConnection_not_ok)
{
  const std::string incorrectIP = "0.0.0.0";
  DBConnection_Mock mockConnection(incorrectIP);
  ClassThatUseDB userDB(&mockConnection);

  EXPECT_CALL(mockConnection, close())
    .Times(1)
    .WillOnce(::testing::Return(false));

  userDB.closeConnection();
}

TEST(ClassThatUseDB, execQuery)
{
  const std::string correctIP = "127.0.0.1";
  DBConnection_Mock mockConnection(correctIP);
  const std::string testQuery = "testQuery";
  EXPECT_CALL(mockConnection, execQuery(testQuery)).Times(1);

  ClassThatUseDB userDB(&mockConnection);
  userDB.useConnection(testQuery);
}



//=============================================================================
int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}