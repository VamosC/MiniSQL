#ifndef ERROR_H
#define ERROR_H

#include <exception>
//���Ѵ����쳣
class TABLE_EXISTED : public std::exception {

};

//�������쳣
class TABLE_NOT_EXISTED : public std::exception {

};
//Ŀ�����Բ������쳣
class ATTR_NOT_EXIST : public std::exception {

};
//where�����е��������ݵ����Ͳ�ƥ���쳣
class WHERE_TYPE_NOT_MATCH : public std::exception {

};
//Tuple���Բ�ƥ���쳣
class TUPLE_ATTR_NOT_MATCH : public std::exception {

};
//������ͻ�쳣
class PRIM_KEY_CONFLICT : public std::exception {

};
//���ݶ������쳣
class UNIQUE_CONFLICT :public std::exception {

};

#endif 