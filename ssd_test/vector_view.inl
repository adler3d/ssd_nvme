template<class TYPE>
struct vector_view{
  const TYPE*p;
  size_t n;
  vector_view<TYPE>():p(nullptr),n(0){}
  const TYPE&operator[](size_t id)const{return p[id];}
  size_t size()const{return n;}
  bool empty()const{return !n;}
  vector_view<TYPE>(const vector<TYPE>&ref,size_t start,size_t n):p(nullptr),n(0)
  {
    if(!n)return;
    QapAssert(qap_check_id(ref,start));
    QapAssert(qap_check_id(ref,start+n-1));
    p=n?&ref[start]:nullptr;
    this->n=n;
  }
  vector_view<TYPE>(const vector<TYPE>&ref):p(nullptr),n(0)
  {
    if(!ref.size())return;
    p=&ref[0];n=ref.size();
  }
  template<size_t N>vector_view(const array<TYPE,N>&ref):p(&ref[0]),n(N){}
  template<size_t N>vector_view<TYPE>(const TYPE(&ref)[N]):p(&ref[0]),n(N){}
};

template<class TYPE>
vector_view<TYPE> make_view(vector<TYPE>&ref,size_t start,size_t n)
{
  vector_view<TYPE> vv;if(!n)return vv;
  QapAssert(qap_check_id(ref,start));
  QapAssert(qap_check_id(ref,start+n-1));
  vv.p=n?&ref[start]:nullptr;
  vv.n=n;
  return vv;
}

template<class TYPE>
const vector_view<TYPE> make_view(const vector<TYPE>&ref,size_t start,size_t n)
{
  vector_view<TYPE> vv;if(!n)return vv;
  if(!qap_check_id(ref,start)||!qap_check_id(ref,start+n-1)){
    #define QAP_SHOW_FLOATS(F)F(ref.size())F(start)F(n)F(start+n-1)
    #include "qap_show_floats.inl"
  }
  vv.p=n?&ref[start]:nullptr;
  vv.n=n;
  return vv;
}

template<class TYPE>
vector_view<TYPE> make_view(vector<TYPE>&ref)
{
  vector_view<TYPE> vv={nullptr,0};if(!ref.size())return vv;
  vv.p=&ref[0];
  vv.n=ref.size();
  return vv;
}

template<class TYPE>
const vector_view<TYPE> make_view(const vector<TYPE>&ref){return ref;}

template<class TYPE,size_t N>
const vector_view<TYPE> make_view(const array<TYPE,N>&ref){return ref;}

template<class TYPE,size_t N>
const vector_view<TYPE> make_view(const TYPE(&ref)[N]){return ref;}