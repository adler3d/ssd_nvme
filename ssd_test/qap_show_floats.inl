#ifndef QAP_SHOW_FLOATS
  static_assert(false,"no way");
  //usage:
  //#define QAP_SHOW_FLOATS(F)F(c.x)F(c.y)F(cx)F(cy)
  //#include "qap_show_floats.inl"
#else
  auto msg=string(
    #define F(VAR)string(#VAR " = "+FToS(VAR)+"\n")+
    QAP_SHOW_FLOATS(F)
    #undef F
    ""
  );
  QapDebugMsg(msg);
  #undef QAP_SHOW_FLOATS
#endif //QAP_SHOW_FLOATS